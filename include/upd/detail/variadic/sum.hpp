#pragma once

#include <utility>

namespace upd::detail::variadic {

template<typename>
struct sum;

template<typename... Integral_Constant_Ts>
struct sum<std::tuple<Integral_Constant_Ts...>> {
  constexpr static auto value = (Integral_Constant_Ts::value + ... + 0);
};

template<typename T>
constexpr auto sum_v = sum<T>::value;

} // namespace upd::detail::variadic
