//! \file
//! \brief SFINAE utilities

#pragma once

#include <boost/type_traits.hpp>
#include <boost/type_traits/is_bounded_array.hpp>

namespace upd {
namespace sfinae {

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

//! \brief Require the provided pack not to be empty
template<typename... Ts>
using require_not_empty_pack = require<sizeof...(Ts) != 0, int>;

} // namespace sfinae
} // namespace upd
