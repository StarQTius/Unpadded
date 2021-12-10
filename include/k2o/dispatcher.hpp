//! \file
//! \brief Reception of order execution request

#pragma once

#include "detail/sfinae.hpp"
#include "keyring.hpp"
#include "order.hpp"
#include "status.hpp"

#include "detail/def.hpp"

namespace k2o {
namespace detail {

//! \brief 'dispatcher' object implementation
//! \note The reason for putting the implementation apart is a GCC compiler bug in C++17 with deduction guide.
template<size_t N>
class dispatcher_impl {
public:
  //! \brief Integer type large enougth to store the order indices
  using index_t = uint16_t;

  //! \brief Function type to unserialize the order indices
  using index_reader_t = index_t(const upd::byte_t *);

  //! \brief Get the functors held by the provided 'keyring' object
  //! \tparam Endianess Endianess used to serialize the values
  //! \tparam Signed_Mode Signed integer representation used to serialize the values
  //! \tparam Ftors... Functors held by the keyring
  template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs &... Ftors>
  explicit dispatcher_impl(keyring<Endianess, Signed_Mode, detail::unevaluated_value_h<Fs, Ftors>...>)
      : orders{order{Ftors, upd::endianess_h<Endianess>{}, upd::signed_mode_h<Signed_Mode>{}}...},
        read_index{static_cast<index_reader_t &>(upd::read_as<index_t, Endianess, Signed_Mode>)} {}

  //! \brief Get the index and arguments from an input byte stream and put the return value into an output byte stream
  //! \param fetch_byte Functor acting as an input byte stream
  //! \param insert_byte Functor acting as an output byte stream
  //! \return The status code returned by the underlying order call
  template<typename Src_F, typename Dest_F>
  status operator()(Src_F &&fetch_byte, Dest_F &&insert_byte) {
    upd::byte_t index_buf[sizeof(index_t)];
    for (auto &byte : index_buf)
      byte = FWD(fetch_byte)();

    return orders[read_index(index_buf)](FWD(fetch_byte), FWD(insert_byte));
  }

private:
  order orders[N];
  index_reader_t &read_index;
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
class dispatcher {
public:
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
  status operator()(Src_F &&fetch_byte, Dest_F &&insert_byte) {
    return m_impl(FWD(fetch_byte), FWD(insert_byte));
  }

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

#include "detail/undef.hpp"
