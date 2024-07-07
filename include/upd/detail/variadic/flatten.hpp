#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../is_instance_of.hpp" // IWYU pragma: keep
#include "clip.hpp"
#include "leaf.hpp"
#include "sum.hpp"

namespace upd::detail::variadic {

template<typename>
struct flatten;

template<typename... Tuple_Ts>
struct flatten<std::tuple<Tuple_Ts...>> {
  static_assert((is_instance_of_v<Tuple_Ts, std::tuple> && ...), "`flatten` only accepts `std::tuple` as arguments");

  using size_ts = std::tuple<std::tuple_size<Tuple_Ts>...>;

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
  [[nodiscard]] constexpr static auto concat(std::index_sequence<Is...>) noexcept {
    return std::tuple<std::remove_pointer_t<decltype(aggregated.at(integral_constant_v<Is>))>...>{};
  }

  constexpr static auto concatenation = concat(std::make_index_sequence<sum_v<size_ts>>{});

  using type = std::remove_const_t<decltype(concatenation)>;
};

template<typename... Tuple_Ts>
using flatten_t = typename flatten<Tuple_Ts...>::type;

} // namespace upd::detail::variadic
