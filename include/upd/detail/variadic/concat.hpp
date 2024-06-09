#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "clip.hpp"
#include "leaf.hpp"
#include "map.hpp"
#include "sum.hpp"

namespace upd::detail::variadic {

template<typename... Tuple_Ts>
struct concat {
  using size_ts = map_t<std::tuple<Tuple_Ts...>, std::tuple_size>;

  template<std::size_t... Is>
  [[nodiscard]] constexpr static auto aggregate(std::index_sequence<Is...>) noexcept {
    struct : decltype(aggregated_leaves_with_offset(std::declval<Tuple_Ts>(),
                                                    std::make_index_sequence<std::tuple_size_v<Tuple_Ts>>{},
                                                    sum<clip_t<size_ts, 0, Is>>{}))... {
      using decltype(aggregated_leaves_with_offset(std::declval<Tuple_Ts>(),
                                                   std::make_index_sequence<std::tuple_size_v<Tuple_Ts>>{},
                                                   sum<clip_t<size_ts, 0, Is>>{}))::at...;
    } retval;

    return retval;
  }

  constexpr static auto aggregated = aggregate(std::index_sequence_for<Tuple_Ts...>{});

  template<std::size_t... Is>
  [[nodiscard]] constexpr static auto flatten(std::index_sequence<Is...>) noexcept {
    return std::tuple<std::remove_pointer_t<decltype(aggregated.at(integral_constant_v<Is>))>...>{};
  }

  constexpr static auto flattened = flatten(std::make_index_sequence<sum_v<size_ts>>{});

  using type = std::remove_const_t<decltype(flattened)>;
};

template<typename... Tuple_Ts>
using concat_t = typename concat<Tuple_Ts...>::type;

} // namespace upd::detail::variadic
