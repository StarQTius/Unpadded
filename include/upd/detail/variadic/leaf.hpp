#pragma once

#include <cstddef>
#include <utility>

#include "../../index_type.hpp"
#include "../../literals.hpp"
#include "upd/detail/range.hpp"

namespace upd::detail::variadic {

template<std::size_t I, typename T>
struct leaf {
  constexpr static auto at(index_type<I>) noexcept -> T *;
  constexpr static auto find(T *) noexcept -> index_type<I>;
};

template<typename... Ts, std::size_t... Is, typename Offset>
[[nodiscard]] constexpr static auto
aggregated_leaves_with_offset(const std::tuple<Ts...> &, std::index_sequence<Is...>, Offset) noexcept {
  constexpr auto offset = Offset::value;

  struct : leaf<offset + Is, Ts>... {
    using leaf<offset + Is, Ts>::at...;
    using leaf<offset + Is, Ts>::find...;
  } retval;

  return retval;
}

template<typename... Ts, std::size_t... Is>
[[nodiscard]] constexpr static auto aggregated_leaves(const std::tuple<Ts...> &leaves,
                                                      std::index_sequence<Is...> seq) noexcept {
  using namespace upd::literals;

  return aggregated_leaves_with_offset(leaves, seq, 0_i);
}

} // namespace upd::detail::variadic
