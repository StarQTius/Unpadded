//! \file

#pragma once

#include <type_traits>

#include "../../format.hpp"

namespace upd {

template<typename Index_T, Index_T, typename, endianess, signed_mode>
struct key;

namespace detail {

//! \brief Check if `T` is a valid key
//! @{

template<typename T>
struct is_key : std::false_type {};
template<typename Index_T, Index_T I, typename R, typename... Args, endianess Endianess, signed_mode Signed_Mode>
struct is_key<key<Index_T, I, R(Args...), Endianess, Signed_Mode>> : std::true_type {};

//! @^}

} // namespace detail
} // namespace upd
