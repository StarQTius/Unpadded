//! \file
//! \brief SFINAE utilities

#pragma once

#include <boost/type_traits/enable_if.hpp>
#include <boost/type_traits/has_plus.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/is_bounded_array.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/type_traits/is_unsigned.hpp>

#include "../format.hpp"

namespace upd {

template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
struct tuple; // IWYU pragma: keep

namespace sfinae {
namespace detail {

//! \brief Indicate if the expression 'upd_extension((T*) nullptr)' is well-formed
template<typename>
constexpr bool is_user_serializable_impl(...) {
  return false;
}
template<typename T, typename = decltype(upd_extension((T *)nullptr))>
constexpr bool is_user_serializable_impl(int) {
  return true;
}

} // namespace detail

//! \brief Indicate if the provided type has an user-defined extension
template<typename T>
struct is_user_serializable : boost::integral_constant<bool, detail::is_user_serializable_impl<T>(0)> {};

//! \brief Require the provided expression to be true
template<bool Expression, typename U = int>
using require = typename boost::enable_if_<Expression, U>::type;

//! \brief Require the provided type to be an unsigned integer type
template<typename T, typename U = int>
using require_unsigned_integer = require<boost::is_unsigned<T>::value, U>;

//! \brief Require the provided type to be a signed integer type
template<typename T, typename U = int>
using require_signed_integer = require<boost::is_signed<T>::value, U>;

//! \brief Require the provided type to be an array bounded type
template<typename T, typename U = int>
using require_bounded_array = require<boost::is_bounded_array<T>::value, U>;

//! \brief Require the provided type to be an array bounded type
template<typename T, typename U = int>
using require_not_bounded_array = require<!boost::is_bounded_array<T>::value, U>;

//! \brief Require the provided type not to be a pointer type
template<typename T, typename U = int>
using require_not_pointer = require<!boost::is_pointer<T>::value, U>;

//! \brief Require the provided type instances to be summable with themselves
template<typename T, typename U = int>
using require_has_plus = require<boost::has_plus<T>::value, U>;

//! \brief Require the provided type instances not to be summable with themselves
template<typename T, typename U = int>
using require_has_not_plus = require<!boost::has_plus<T>::value, U>;

//! \brief Require the provided pack not to be empty
template<typename... Ts>
using require_not_empty_pack = require<sizeof...(Ts) != 0, int>;

//! \brief Require the provided type to have an user-defined extension
template<typename T, typename U = int>
using require_is_user_serializable = require<is_user_serializable<T>::value, U>;

//! \brief Always fail
template<typename T, typename U = int>
using fail = require<boost::integral_constant<decltype((T *)nullptr, bool{}), false>::value, U>;

} // namespace sfinae
} // namespace upd
