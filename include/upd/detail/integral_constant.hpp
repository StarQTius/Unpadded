#pragma once

#include <tuple>
#include <type_traits>

namespace upd::detail {

template<auto Value>
using integral_constant_t = std::integral_constant<decltype(Value), Value>;

template<auto... Values>
using integral_constant_tuple_t = std::tuple<integral_constant_t<Values>...>;

template<auto X>
constexpr auto integral_constant_v = integral_constant_t<X>{};

} // namespace upd::detail
