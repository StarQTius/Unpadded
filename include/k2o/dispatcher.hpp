//! \file
//! \brief Order request immediate processing

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

#include <tl/expected.hpp>
#include <upd/format.hpp>
#include <upd/type.hpp>

#include "detail/sfinae.hpp"
#include "detail/value_h.hpp" // IWYU pragma: keep
#include "keyring.hpp"
#include "order.hpp"

#include "detail/def.hpp"

// IWYU pragma: no_forward_declare unevaluated_value_h

namespace k2o {
namespace detail {

//! \brief Extract the index from a byte sequence
template<typename Src_F, typename Index>
Index get_index(Src_F &&fetch_byte, Index (&read_index)(const upd::byte_t *)) {
  upd::byte_t index_buf[sizeof(Index)];
  for (auto &byte : index_buf)
    byte = FWD(fetch_byte)();

  return read_index(index_buf);
}

//! \brief 'dispatcher' object implementation
//! \note The reason for putting the implementation apart is a GCC compiler bug in C++17 with deduction guide.
template<size_t N>
struct dispatcher_impl {
  //! \brief Integer type large enougth to store the order indices
  using index_t = uint16_t;

  //! \brief Function type to unserialize the order indices
  using index_reader_t = index_t(const upd::byte_t *);

  order orders[N];
  index_reader_t &read_index;

  //! \brief Get the functors held by the provided 'keyring' object
  //! \tparam Endianess Endianess used to serialize the values
  //! \tparam Signed_Mode Signed integer representation used to serialize the values
  //! \tparam Ftors... Functors held by the keyring
  template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs &...Ftors>
  explicit dispatcher_impl(keyring<Endianess, Signed_Mode, detail::unevaluated_value_h<Fs, Ftors>...>)
      : orders{order{Ftors, upd::endianess_h<Endianess>{}, upd::signed_mode_h<Signed_Mode>{}}...},
        read_index{static_cast<index_reader_t &>(upd::read_as<index_t, Endianess, Signed_Mode>)} {}

  //! \brief Get the index and arguments from an input byte stream and call the matching order
  //! \details
  //!   The return value is inserted into the output byte stream. If the index is out of bound, no call are performed.
  //! \param fetch_byte Functor acting as an input byte stream
  //! \param insert_byte Functor acting as an output byte stream
  //! \return The index got from the input byte stream
  template<typename Src_F, typename Dest_F>
  index_t operator()(Src_F &&fetch_byte, Dest_F &&insert_byte) {
    auto index = get_index(fetch_byte);

    if (index < N)
      orders[index](fetch_byte, insert_byte);

    return index;
  }

  //! \brief Get the order indicated by the byte sequence
  template<typename Src_F>
  tl::expected<std::reference_wrapper<order>, index_t> get_order(Src_F &&fetch_byte) {
    using return_t = tl::expected<std::reference_wrapper<order>, index_t>;

    auto index = get_index(FWD(fetch_byte));
    return index < N ? return_t{orders[index]} : tl::make_unexpected(index);
  }

  template<typename Src_F>
  index_t get_index(Src_F &&fetch_byte) const {
    return detail::get_index(FWD(fetch_byte), read_index);
  }
};

} // namespace detail

//! \brief Order containers able to unserialize byte sequence serialized by an 'key' object
//! \details
//!   A 'dispatcher' object is constructed from a 'keyring' object and is able to unserialize a payload serialized by
//!   an 'key' object created from the same 'keyring' object. When it happens, the 'dispatcher' object calls the
//!   function associated with the 'key' object, forwarding the arguments from the payload to the function. The
//!   functions are internally held as 'order' objects.
//! \tparam N the number of functions held by the 'keyring' object
template<size_t N>
struct dispatcher {
  using index_t = typename detail::dispatcher_impl<N>::index_t;
  constexpr static auto size = N;

  //! \brief Construct the object from the provided 'keyring' object
  //! \details
  //!   The 'N' template parameter can be deduced from the number of functions held by the 'keyring' object.
  //! \param input_keyring 'keyring' object
  //! \note
  //!   The strange structure of this function is due to a GCC compiler bug with deduction guide in C++17.
  template<typename T, sfinae::require_is_deriving_from_keyring<T> = 0>
  explicit dispatcher(T input_keyring) : m_impl{input_keyring} {}

  //! \brief Call the function according to the index and arguments obtained from a payload
  //! \param fetch_byte functor behaving as an input byte stream, from which the payload is fetched
  //! \param insert_byte functor behaving as an output byte stream, in which the function call return value will be put
  //! \return the status code obtained from the underlying order call
  template<typename Src_F, typename Dest_F>
  typename detail::dispatcher_impl<N>::index_t operator()(Src_F &&fetch_byte, Dest_F &&insert_byte) {
    return m_impl(FWD(fetch_byte), FWD(insert_byte));
  }

  //! \brief Get the order indicated by the byte sequence
  //! \param fetch_byte Functor acting as an input byte stream
  //! \return Either a reference to the order if it exists or the index that obtained from the byte sequence
  template<typename Src_F>
  tl::expected<std::reference_wrapper<order>, index_t> get_order(Src_F &&fetch_byte) {
    return m_impl.get_order(FWD(fetch_byte));
  }

  //! \copydoc dispatcher_impl::get_index
  //! \param fetch_byte Input functor to a byte sequence
  template<typename Src_F>
  index_t get_index(Src_F &&fetch_byte) const {
    return m_impl.get_index(FWD(fetch_byte));
  }

  //! \brief Get one of the stored orders
  //! \param index Index of an order
  //! \return the order associated with that index
  //! \warning There are no bound check performed.
  order &operator[](index_t index) { return m_impl.orders[index]; }

  //! \copydoc operator[]
  const order &operator[](index_t index) const { return m_impl.orders[index]; }

private:
  detail::dispatcher_impl<N> m_impl;
};

#if __cplusplus >= 201703L
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs... Ftors>
dispatcher(keyring<Endianess, Signed_Mode, detail::unevaluated_value_h<Fs, Ftors>...>) -> dispatcher<sizeof...(Fs)>;
#endif // __cplusplus >= 201703L

//! \brief Make a 'dispatcher' object
//! \param input_keyring 'keyring' object forwarded to the 'dispatcher' constructor
//! \return a 'dispatcher' object whose orders calls the functors held by 'input_keyring'
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs... Ftors>
dispatcher<sizeof...(Fs)>
make_dispatcher(keyring<Endianess, Signed_Mode, detail::unevaluated_value_h<Fs, Ftors>...> input_keyring) {
  return dispatcher<sizeof...(Fs)>{input_keyring};
}

} // namespace k2o

#include "detail/undef.hpp" // IWYU pragma: keep
