#pragma once

#include <type_traits>

namespace upd::detail::variadic {

template<typename>
struct all_of;

template<typename... Integral_Constant_Ts>
struct all_of<std::tuple<Integral_Constant_Ts...>> {
  constexpr static auto value = (Integral_Constant_Ts::value && ... && true);
};

template<typename T>
constexpr auto all_of_v = all_of<T>::value;

} // namespace upd::detail::variadic