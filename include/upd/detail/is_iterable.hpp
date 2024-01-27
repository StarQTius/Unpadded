#pragma once

#include <iterator> // IWYU pragma: keep
#include <type_traits>

namespace upd::detail {

template<typename, typename = void>
struct is_iterable : std::false_type {};

template<typename T>
struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>>
    : std::true_type {};

template<typename T>
constexpr inline bool is_iterable_v = is_iterable<T>::value;

} // namespace upd::detail
