//! \file
//! \brief Template overloaded function disambiguation

#pragma once

#include <boost/type_traits/enable_if.hpp>
#include <boost/type_traits/integral_constant.hpp>

#include <upd/format.hpp>
#include <upd/tuple.hpp>

#include "value_h.hpp"

namespace k2o {

// Forward declaration
template<upd::endianess, upd::signed_mode, typename...>
class keyring;

namespace sfinae {
namespace detail {

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs... Ftors>
constexpr boost::true_type
    is_deriving_from_keyring_impl(keyring<Endianess, Signed_Mode, k2o::detail::unevaluated_value_h<Fs, Ftors>...>);
constexpr boost::false_type is_deriving_from_keyring_impl(...);

} // namespace detail

//! \brief Assert if the provided type is a template instance of 'upd::tuple'
template<typename>
struct is_tuple : boost::false_type {};
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct is_tuple<upd::tuple<Endianess, Signed_Mode, Ts...>> : boost::true_type {};

//! \brief Check if the provided type derives from 'keyring'
template<typename T>
struct is_deriving_from_keyring : decltype(detail::is_deriving_from_keyring_impl(boost::declval<T>())) {};

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

//! \brief Require the provided type to derive from 'keyring'
template<typename T, typename U = int>
using require_is_deriving_from_keyring = require<is_deriving_from_keyring<T>::value, U>;

} // namespace sfinae
} // namespace k2o
