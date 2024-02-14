#pragma once

#include <algorithm> // IWYU pragma: keep
#include <tuple>

namespace upd::detail::variadic {

template<typename>
struct max;

template<typename... Integral_Constant_Ts>
struct max<std::tuple<Integral_Constant_Ts...>> {
  constexpr static auto value = std::max({Integral_Constant_Ts::value...});
};

template<typename T>
constexpr auto max_v = max<T>::value;

} // namespace upd::detail::variadic
