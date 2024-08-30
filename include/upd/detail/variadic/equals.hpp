#pragma once

namespace upd::detail::variadic {

template<typename, typename>
struct equals;

template<typename... Ts, typename... Us>
struct equals<std::tuple<Ts...>, std::tuple<Us...>>: std::bool_constant<((Ts::value == Us::value) && ...)> {};

template<typename Lhs, typename Rhs>
constexpr auto equals_v = equals<Lhs, Rhs>::value;

} // namespace upd::detail::variadic