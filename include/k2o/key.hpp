//! \file
//! \brief Order execution at sender side

#pragma once

#include <boost/type_traits.hpp>

#include <k2o/detail/fwd.hpp>
#include <k2o/detail/signature.hpp>

#include <upd/storage/tuple.hpp>

namespace k2o {
namespace detail {

//! \brief Simple wrapper around 'upd::tuple' whose content can be forwarded to a functor as a byte sequence
//! \detail
//!   The content can be forwarded with the 'operator>>' function member. 'detail::serialized_message' object
//!   cannot be copied from to avoid unintentional copy.
//! \tparam Endianess forwarded to 'upd::tuple'
//! \tparam Signed_Mode forwarded to 'upd::tuple'
//! \tparam Ts... forwarded to 'upd::tuple'
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct serialized_message {
  serialized_message(const Ts &... values) : content{values...} {}

  serialized_message(const serialized_message &) = delete;
  serialized_message(serialized_message &&) = default;

  serialized_message &operator=(const serialized_message &) = delete;
  serialized_message &operator=(serialized_message &&) = default;

  template<typename Dest_F>
  void operator>>(Dest_F &&insert_byte) const {
    for (auto byte : content)
      K2O_FWD(insert_byte)(byte);
  }

  upd::tuple<Endianess, Signed_Mode, Ts...> content;
};

//! \brief Helper class that allows a 'key' object to introduce endianess and signed integer representation
//! \tparam Endianess endianess specified by the 'key' object
//! \tparam Signed_Mode signed integer representation specified by the 'key' object
//! \tparam R if unserializing, type of the result
//! \tparam Args... if serializing, type of the parameters
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename R, typename... Args>
struct serialized_message_maker {
  serialized_message<Endianess, Signed_Mode, boost::remove_cv_ref_t<Args>...> operator()(const Args &... args) const {
    return {args...};
  }

  template<typename Src_F>
  R operator<<(Src_F &&fetch_byte) const {
    upd::tuple<Endianess, Signed_Mode, R> retval;
    for (auto &byte : retval)
      byte = K2O_FWD(fetch_byte)();

    return retval.template get<0>();
  }
};

} // namespace detail

//! \brief Holds endianess and signed integer representation information
//! \tparam Endianess endianess information
//! \tparam Signed_Mode signed integer representation information
template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct profile {};

template<typename F>
class key : public key<detail::signature_t<F>> {};

//! \brief Serializing / unserializing helper class according to a function signature
//! \tparam R return type associated with the signature
//! \tparam Args... argument types associated with the signature
//! \tparam Endianess considered endianess when serializing/unserializing arguments
//! \tparam Signed_Mode considered signed integer representation when serializing/unserializing arguments
template<typename R, typename... Args>
class key<R(Args...)> {
public:
  //! \brief Create a 'serialized_message_maker' object
  //! \detail
  //!   This allows the following syntax : 'key[profile](x1, x2, x3, ...) >> dest_f' (with 'dest_f' being a functor of
  //!   signature 'void(byte_t)'). 'dest_f' is called with every byte representing the data passed as parameter.
  //!   Analogously, an order result can be fetched with the following syntax : 'auto result = key[profile] << src_f'
  //!   (with 'src_f' being a functor of signature 'byte_t()').
  //! \tparam Endianess considered endianess for serializing / unserializing
  //! \tparam Signed_Mode considered signed integer representation for serializing / unserializing
  //! \return a 'serialized_message' object holding the arguments passed as parameters
  template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
  detail::serialized_message_maker<Endianess, Signed_Mode, R, Args...> operator[](profile<Endianess, Signed_Mode>) {
    return {};
  }

  //! \brief Create a 'serialized_message' object
  //! \detail
  //!   This allows the following syntax : 'key(x1, x2, x3, ...) >> dest_f' (with 'dest_f' being a functor of signature
  //!   'void(byte_t)'). 'dest_f' is called with every byte representing the data passed as parameter.
  //! \param args... arguments to be serialized
  //! \return a 'serialized_message' object holding the arguments passed as parameters
  detail::serialized_message<upd::endianess::BUILTIN, upd::signed_mode::BUILTIN, boost::remove_cv_ref_t<Args>...>
  operator()(const Args &... args) const {
    return {args...};
  }

  //! \brief Unserialize a value from an input byte stream
  //! \detail
  //!   The input byte stream is provided through the functor passed as parameter. When called with no parameters, it
  //!   must return a 'byte_t' value.
  //! \param fetch_byte Functor acting as a input byte stream
  //! \return the unserialized value
  template<typename Src_F>
  R operator<<(Src_F &&fetch_byte) const {
    upd::tuple<upd::endianess::BUILTIN, upd::signed_mode::BUILTIN, R> retval;
    for (auto &byte : retval)
      byte = K2O_FWD(fetch_byte)();

    return retval.template get<0>();
  }
};

#if __cplusplus >= 201703L
//! \brief (C++17) Create a 'key' object
//! \detail
//!   This function use C++17 placeholder auto with template non-type parameter to allow nicer syntax.
//! \tparam Function function whose signature will be passed to the returned object
//! \return 'key<boost::remove_cv_ref_t<decltype(*Function)>>{}'
template<auto &Function>
constexpr auto make_key() {
  return key<boost::remove_cv_ref_t<decltype(Function)>>{};
}
#endif // __cplusplus >= 201703L

} // namespace k2o
