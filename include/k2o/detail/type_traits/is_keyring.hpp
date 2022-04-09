//! \file

#pragma once

#include <type_traits>

#include <upd/format.hpp>

#include "../../unevaluated.hpp" // IWYU pragma: keep

// IWYU pragma: no_forward_declare unevaluated

namespace k2o {

template<upd::endianess, upd::signed_mode, typename...>
class keyring;

namespace detail {

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs... Ftors>
std::true_type is_keyring_impl(keyring<Endianess, Signed_Mode, k2o::unevaluated<Fs, Ftors>...>);
std::false_type is_keyring_impl(...);

//! \brief Check if `T` is a valid keyring
template<typename T>
struct is_keyring : decltype(is_keyring_impl(std::declval<T>())) {};

} // namespace detail
} // namespace k2o
