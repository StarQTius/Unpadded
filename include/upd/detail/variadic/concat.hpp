#pragma once

#include <tuple>

#include "flatten.hpp"

namespace upd::detail::variadic {

template<typename... Tuple_Ts>
struct concat {
  static_assert((is_instance_of_v<Tuple_Ts, std::tuple> && ...), "`flatten` only accepts `std::tuple` as arguments");

  using tuple_ts = std::tuple<Tuple_Ts...>;
  using type = flatten_t<tuple_ts>;
};

template<typename... Tuple_Ts>
using concat_t = typename concat<Tuple_Ts...>::type;

} // namespace upd::detail::variadic
