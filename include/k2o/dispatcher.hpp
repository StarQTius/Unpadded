//! \file
//! \brief Reception of order execution request

#pragma once

#include "detail/sfinae.hpp"
#include "keyring11.hpp"
#include "order.hpp"
#include "status.hpp"

#include "detail/def.hpp"

namespace k2o {
namespace detail {

//! \brief 'dispatcher' object implementation
//! \note The reason for putting the implementation apart is a GCC compiler bug in C++17 with deduction guide.
template<size_t N>
struct dispatcher_impl {
  //! \brief Initialize 'orders' with the functors held by the provided 'keyring11' object
  template<typename... Fs, Fs &... Ftors>
  explicit dispatcher_impl(keyring11<detail::unevaluated_value_h<Fs, Ftors>...>) : orders{order{Ftors}...} {}

  //! \brief Get the index and arguments from an input byte stream and put the return value into an output byte stream
  template<typename Src_F, typename Dest_F>
  status operator()(Src_F &&fetch_byte, Dest_F &&insert_byte) {
    using namespace upd;

    tuple<endianess::BUILTIN, signed_mode::BUILTIN, uint16_t> order_index;
    for (auto &byte : order_index)
      byte = FWD(fetch_byte)();

    return orders[order_index.get<0>()](FWD(fetch_byte), FWD(insert_byte));
  }

  order orders[N];
};

} // namespace detail

//! \brief Order containers able to unserialize byte sequence serialized by an 'ikey' object
//! \details
//!   A 'dispatcher' object is constructed from a 'keyring11' object and is able to unserialize a payload serialized by
//!   an 'ikey' object created from the same 'keyring11' object. When it happens, the 'dispatcher' object calls the
//!   function associated with the 'ikey' object, forwarding the arguments from the payload to the function. The
//!   functions are internally held as 'order' objects.
//! \tparam N the number of functions held by the 'keyring11' object
template<size_t N>
class dispatcher {
public:
  //! \brief Construct the object from the provided 'keyring11' object
  //! \details
  //!   The 'N' template parameter can be deduced from the number of functions held by the 'keyring11' object.
  //! \param input_keyring 'keyring11' object
  //! \note
  //!   The strange structure of this function is due to a GCC compiler bug with deduction guide in C++17.
  template<typename T, sfinae::require_is_deriving_from_keyring11<T> = 0>
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
template<typename... Fs, Fs... Ftors>
dispatcher(keyring11<detail::unevaluated_value_h<Fs, Ftors>...>) -> dispatcher<sizeof...(Fs)>;
#endif // __cplusplus >= 201703L

//! \brief Make a 'dispatcher' object
//! \param input_keyring 'keyring11' object whose held functors will be called by the returned 'dispatcher' object
//! \return a 'dispatcher' object whose orders calls the functors held by 'input_keyring'
template<typename... Fs, Fs... Ftors>
dispatcher<sizeof...(Fs)> make_dispatcher(keyring11<detail::unevaluated_value_h<Fs, Ftors>...> input_keyring) {
  return dispatcher<sizeof...(Fs)>{input_keyring};
}

} // namespace k2o

#include "detail/undef.hpp"
