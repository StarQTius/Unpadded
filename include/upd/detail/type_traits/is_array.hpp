//! \file

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

namespace upd {
namespace detail {

//! \brief Check is `A` is an array (C-style or `std::array` instance)
//! @{

template<typename A>
struct is_array : std::false_type {};
template<typename T, std::size_t N>
struct is_array<T[N]> : std::true_type {};
template<typename T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type {};

//! @}

} // namespace detail
} // namespace upd
