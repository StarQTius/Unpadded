//! \file
//! \brief Order execution at sender side

#pragma once

#include <boost/type_traits.hpp>

#include <k2o/detail/fwd.hpp>

#include <upd/storage/tuple.hpp>

namespace k2o {
namespace detail {

//! \brief Simple wrapper around 'upd::tuple' whose content can be forwarded to a functor as a byte sequence
//! \detail
//!   The content can be forwarded with the 'operator>>' function member. 'detail::serialized_data' object cannot be
//!   copied from to avoid unintentional copy.
//! \tparam Endianess forwarded to 'upd::tuple'
//! \tparam Signed_Mode forwarded to 'upd::tuple'
//! \tparam Ts... forwarded to 'upd::tuple'
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct serialized_data {
  serialized_data(const Ts &... values) : content{values...} {}

  serialized_data(const serialized_data &) = delete;
  serialized_data(serialized_data &&) = default;

  serialized_data &operator=(const serialized_data &) = delete;
  serialized_data &operator=(serialized_data &&) = default;

  template<typename Dest_F>
  void operator>>(Dest_F &&insert_byte) const {
    for (auto byte : content)
      K2O_FWD(insert_byte)(byte);
  }

  upd::tuple<Endianess, Signed_Mode, Ts...> content;
};

} // namespace detail

template<typename F, upd::endianess = upd::endianess::BUILTIN, upd::signed_mode = upd::signed_mode::BUILTIN>
class key;

//! \brief Serializing / unserializing helper class according to a function signature
//! \tparam R return type associated with the signature
//! \tparam Args... argument types associated with the signature
//! \tparam Endianess considered endianess when serializing/unserializing arguments
//! \tparam Signed_Mode considered signed integer representation when serializing/unserializing arguments
template<typename R, typename... Args, upd::endianess Endianess, upd::signed_mode Signed_Mode>
class key<R(Args...), Endianess, Signed_Mode> {
public:
  //! \brief Create a 'serialized_data' object
  //! \detail
  //!   This allows the following syntax : 'key(x1, x2, x3, ...) >> dest_f' (with 'dest_f' being a functor of signature
  //!   'void(byte_t)'). 'dest_f' is called with every byte representing the data passed as parameter.
  //! \param args... arguments to be serialized
  //! \return a 'serialized_data' object holding the arguments passed as parameters
  detail::serialized_data<Endianess, Signed_Mode, boost::remove_cv_ref_t<Args>...>
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
    upd::tuple<Endianess, Signed_Mode, R> retval;
    for (auto &byte : retval)
      byte = K2O_FWD(fetch_byte)();

    return retval.template get<0>();
  }
};

} // namespace k2o
