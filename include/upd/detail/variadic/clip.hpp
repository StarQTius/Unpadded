#pragma once

#include <cstddef>
#include <utility>

#include "at.hpp"

namespace upd::detail::variadic {

template<typename, std::size_t, std::size_t>
struct clip;

template<typename... Ts, std::size_t B, std::size_t E>
struct clip<std::tuple<Ts...>, B, E> {
  static_assert(B <= E);

  template<std::size_t... Is>
  [[nodiscard]] constexpr static auto clipped_tuple(std::index_sequence<Is...>) noexcept {
    return std::tuple<at_t<std::tuple<Ts...>, B + Is>...>{};
  }

  constexpr static std::make_index_sequence<E - B> iseq{};
  using type = decltype(clipped_tuple(iseq));
};

template<typename T, std::size_t B, std::size_t E>
using clip_t = typename clip<T, B, E>::type;

} // namespace upd::detail::variadic
