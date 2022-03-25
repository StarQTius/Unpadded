//! \file
//! \brief Order request immediate processing

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

#include <tl/expected.hpp>
#include <upd/format.hpp>
#include <upd/type.hpp>

#include "detail/io.hpp"
#include "detail/sfinae.hpp"
#include "detail/value_h.hpp" // IWYU pragma: keep
#include "keyring.hpp"
#include "order.hpp"

#include "detail/def.hpp"

// IWYU pragma: no_forward_declare unevaluated_value_h

namespace k2o {
namespace detail {

//! \brief Extract the index from a byte sequence
template<typename Src, typename Index>
Index get_index(Src &&src, Index (&read_index)(const upd::byte_t *)) {
  upd::byte_t index_buf[sizeof(Index)];
  for (auto &byte : index_buf)
    byte = FWD(src)();

  return read_index(index_buf);
}

//! \brief Dispatcher implementation
//! \note The reason for putting the implementation apart is a GCC compiler bug in C++17 with deduction guide.
template<size_t N>
struct dispatcher_impl {
  //! \brief Integer type large enougth to store the order indices
  using index_t = uint16_t;

  //! \brief Function type to unserialize the order indices
  using index_reader_t = index_t(const upd::byte_t *);

  order orders[N];
  index_reader_t &read_index;

  //! \brief Get the functors held by the provided keyring
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
  //! \param src Functor acting as an input byte stream
  //! \param dest Functor acting as an output byte stream
  //! \return The index got from the input byte stream
  template<typename Src, typename Dest>
  index_t operator()(Src &&src, Dest &&dest) {
    auto index = get_index(src);

    if (index < N)
      orders[index](src, dest);

    return index;
  }

  //! \brief Get the order indicated by the byte sequence
  template<typename Src>
  tl::expected<std::reference_wrapper<order>, index_t> get_order(Src &&src) {
    using return_t = tl::expected<std::reference_wrapper<order>, index_t>;

    auto index = get_index(FWD(src));
    return index < N ? return_t{orders[index]} : tl::make_unexpected(index);
  }

  template<typename Src>
  index_t get_index(Src &&src) const {
    return detail::get_index(FWD(src), read_index);
  }
};

} // namespace detail

//! \brief Order containers able to unserialize byte sequence serialized by an key
//! \details
//!   A dispatcher is constructed from a keyring and is able to unserialize a payload serialized by
//!   an key created from the same keyring. When it happens, the dispatcher calls the
//!   function associated with the key, forwarding the arguments from the payload to the function. The
//!   functions are internally held as 'order' objects.
//! \tparam N the number of functions held by the keyring
template<size_t N>
class dispatcher : public detail::immediate_process<dispatcher<N>, typename detail::dispatcher_impl<N>::index_t> {
public:
  using index_t = typename detail::dispatcher_impl<N>::index_t;
  constexpr static auto size = N;

  //! \brief Construct the object from the provided keyring
  //! \details
  //!   'N' can be deduced from the number of functions held by the keyring.
  //! \param kring Keyring containing the orders to dispatch to
  //! \note
  //!   The strange structure of this function is due to a GCC compiler bug with deduction guide in C++17.
  template<typename Keyring, sfinae::require_is_deriving_from_keyring<Keyring> = 0>
  explicit dispatcher(Keyring kring) : m_impl{kring} {}

  using detail::immediate_process<dispatcher<N>, index_t>::operator();

  //! \brief Call the function according to the index and arguments obtained from a payload
  //! \param src functor behaving as an input byte stream, from which the payload is fetched
  //! \param dest functor behaving as an output byte stream, in which the function call return value will be put
  //! \return the index of the called order
  template<typename Src, typename Dest, sfinae::require_input_ftor<Src> = 0, sfinae::require_output_ftor<Dest> = 0>
  index_t operator()(Src &&src, Dest &&dest) {
    return m_impl(FWD(src), FWD(dest));
  }

  //! \brief Get the order indicated by the byte sequence
  //! \param src Functor acting as an input byte stream
  //! \return Either a reference to the order if it exists or the index that obtained from the byte sequence
  template<typename Src, sfinae::require_input_ftor<Src> = 0>
  tl::expected<std::reference_wrapper<order>, index_t> get_order(Src &&src) {
    return m_impl.get_order(FWD(src));
  }

  //! \copydoc dispatcher_impl::get_index
  //! \param src Input functor to a byte sequence
  template<typename Src, sfinae::require_input_ftor<Src> = 0>
  index_t get_index(Src &&src) const {
    return m_impl.get_index(FWD(src));
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

//! \brief Make a dispatcher
//! \param kring keyring forwarded to the dispatcher
//! \return a dispatcher whose orders calls the functors held by the keyring
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs... Ftors>
dispatcher<sizeof...(Fs)>
make_dispatcher(keyring<Endianess, Signed_Mode, detail::unevaluated_value_h<Fs, Ftors>...> kring) {
  return dispatcher<sizeof...(Fs)>{kring};
}

} // namespace k2o

#include "detail/undef.hpp" // IWYU pragma: keep
