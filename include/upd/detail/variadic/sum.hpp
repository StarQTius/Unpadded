#pragma once

#include <type_traits>
#include <utility>

namespace upd::detail::variadic {

template<typename>
struct sum;

template<typename... Ts, Ts... Values>
struct sum<std::tuple<std::integral_constant<Ts, Values>...>> {
  constexpr static auto value = (Values + ...);
};

template<typename T>
constexpr auto sum_v = sum<T>::value;

} // namespace upd::detail::variadic
