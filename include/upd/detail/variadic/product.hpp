#pragma once

#include <tuple>
#include <utility>

#include "flatten.hpp"
#include "map.hpp"

namespace upd::detail::variadic {

template<typename, typename>
struct product;

template<typename... Ts, typename... Us>
struct product<std::tuple<Ts...>, std::tuple<Us...>> {
  using tuple_matrix_t = std::tuple<map_t<std::tuple<Ts...>, std::pair, Us>...>;

  using type = flatten_t<tuple_matrix_t>;
};

template<typename Lhs_Ts, typename Rhs_Ts>
using product_t = typename product<Lhs_Ts, Rhs_Ts>::type;

} // namespace upd::detail::variadic
