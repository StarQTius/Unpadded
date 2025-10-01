#pragma once

#include <type_traits>

#include "../upd.hpp"
#include "side.hpp"

namespace upd::algebra {

template<typename Lhs, typename Rhs>
struct divide {
  Lhs lhs;
  Rhs rhs;
};

template<typename Lhs, typename Rhs, typename Lets>
[[nodiscard]] constexpr auto calculate(const divide<Lhs, Rhs> &expr, const Lets &lets) noexcept(release) {
  return calculate(expr.lhs, lets) / calculate(expr.rhs, lets);
}

template<typename Lhs, typename Rhs>
[[nodiscard]] constexpr auto operator/(const side<Lhs> &lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{divide{lhs.expr, rhs.expr}};
}

template<typename Lhs, typename Rhs>
  requires std::is_arithmetic_v<Rhs>
[[nodiscard]] constexpr auto operator/(const side<Lhs> &lhs, Rhs rhs) noexcept(release) {
  return side{divide{lhs.expr, rhs}};
}

template<typename Lhs, typename Rhs>
  requires std::is_arithmetic_v<Lhs>
[[nodiscard]] constexpr auto operator/(Lhs lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{divide{lhs, rhs.expr}};
}

} // namespace upd::algebra
