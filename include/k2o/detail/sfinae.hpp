#pragma once

#include <boost/type_traits/enable_if.hpp>
#include <boost/type_traits/integral_constant.hpp>

/*!
  \file
*/

namespace k2o {
namespace sfinae {

/*!
 */
template<typename>
struct is_tuple : boost::false_type {};
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct is_tuple<upd::tuple<Endianess, Signed_Mode, Ts...>> : boost::true_type {};

//!
template<typename T, typename U = int>
using require_is_tuple = typename boost::enable_if_<is_tuple<T>::value, U>::type;

//!
template<typename T, typename U = int>
using require_not_tuple = typename boost::enable_if_<!is_tuple<T>::value, U>::type;

//!
template<typename T, typename U = int>
using require_is_void = typename boost::enable_if_<boost::is_void<T>::value, U>::type;

//!
template<typename T, typename U = int>
using require_not_void = typename boost::enable_if_<!boost::is_void<T>::value, U>::type;

} // namespace sfinae
} // namespace k2o
