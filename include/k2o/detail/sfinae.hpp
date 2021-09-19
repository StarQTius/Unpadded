//! \file
//! \brief Template overloaded function disambiguation

#pragma once

#include <boost/type_traits/enable_if.hpp>
#include <boost/type_traits/integral_constant.hpp>

#include <upd/format.hpp>
#include <upd/storage/tuple.hpp>

namespace k2o {
namespace sfinae {

//! \brief Assert if the provided type is a template instance of 'upd::tuple'
template<typename>
struct is_tuple : boost::false_type {};
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct is_tuple<upd::tuple<Endianess, Signed_Mode, Ts...>> : boost::true_type {};

//! \brief Require the provided expression to be true
template<bool Expression, typename U = int>
using require = typename boost::enable_if_<Expression, U>::type;

//! \brief Require the provided type to be a template instance of 'upd::tuple'
template<typename T, typename U = int>
using require_is_tuple = require<is_tuple<T>::value, U>;

//! \brief Require the provided type not to be a template instance of 'upd::tuple'
template<typename T, typename U = int>
using require_not_tuple = require<!is_tuple<T>::value, U>;

//! \brief Require the provided type to be 'void'
template<typename T, typename U = int>
using require_is_void = require<boost::is_void<T>::value, U>;

//! \brief Require the provided type not to be 'void'
template<typename T, typename U = int>
using require_not_void = require<!boost::is_void<T>::value, U>;

//! \brief Require the provided type to be a function type
template<typename T, typename U = int>
using require_is_function = require<boost::is_function<boost::remove_pointer_t<boost::decay_t<T>>>::value, U>;

} // namespace sfinae
} // namespace k2o
