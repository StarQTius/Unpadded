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
  using type = std::remove_pointer_t<decltype(aggregated_leaves(std::declval<std::tuple<Ts...>>(),
                                                                std::make_index_sequence<sizeof...(Ts)>{})
                                                  .at(index_type_v<I>))>;
};

template<typename T, std::size_t I>
using at_t = typename at<T, I>::type;

} // namespace upd::detail::variadic
