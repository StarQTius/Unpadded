#pragma once

#include <cstddef>
#include <type_traits>

namespace upd::detail {

template<typename>
struct is_bounded_array : std::false_type {};

template<typename T, std::size_t N>
struct is_bounded_array<T[N]> : std::true_type {};

template<typename T>
constexpr auto is_bounded_array_v = is_bounded_array<T>::value;

} // namespace upd::detail
