#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "leaf.hpp"

namespace upd::detail::variadic {

template<typename, std::size_t>
struct at;

template<typename... Ts, std::size_t I>
struct at<std::tuple<Ts...>, I> {
  template<std::size_t... Is>
  [[nodiscard]] constexpr static auto aggregated_leaves(std::index_sequence<Is...>) {
    struct : leaf<Is, Ts>... {
      using leaf<Is, Ts>::at...;
      using leaf<Is, Ts>::find...;
    } retval;

    return retval;
  }

  using type =
      std::remove_pointer_t<decltype(aggregated_leaves(std::make_index_sequence<sizeof...(Ts)>{}).at(index_type_v<I>))>;
};

template<typename T, std::size_t I>
using at_t = typename at<T, I>::type;

} // namespace upd::detail::variadic
