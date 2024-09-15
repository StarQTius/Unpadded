#pragma once

namespace upd::detail::variadic {

template<typename, typename>
struct equals;

template<typename... Ts, typename... Us>
struct equals<std::tuple<Ts...>, std::tuple<Us...>> {
  constexpr static auto value = [] {
    if constexpr (sizeof...(Ts) == sizeof...(Us)) {
      return ((Ts::value == Us::value) && ...);
    } else {
      return false;
    }
  }();
};

template<typename Lhs, typename Rhs>
constexpr auto equals_v = equals<Lhs, Rhs>::value;

} // namespace upd::detail::variadic