#pragma once

#include <type_traits>
#include <utility>

#include "leaf.hpp"

namespace upd::detail::variadic {

template<typename, typename>
struct find;

template<typename... Ts, typename U>
struct find<std::tuple<Ts...>, U>
    : decltype(aggregated_leaves(std::declval<std::tuple<Ts...>>(), std::make_index_sequence<sizeof...(Ts)>{})
                   .find((U *)nullptr)) {};

template<typename T, typename U>
constexpr auto find_v = find<T, U>::value;

} // namespace upd::detail::variadic
