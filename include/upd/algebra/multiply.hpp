#pragma once

#include <type_traits>

#include "../record/concepts.hpp"
#include "../upd.hpp"
#include "concepts.hpp"
#include "side.hpp"

namespace upd::algebra {

template<expression Lhs, expression Rhs>
struct multiply {
  Lhs lhs;
  Rhs rhs;
};

template<expression Lhs, expression Rhs, record_like Lets>
[[nodiscard]] constexpr auto substitute(const multiply<Lhs, Rhs> &expr, const Lets &lets) noexcept(release) {
  return substitute(expr.lhs, lets) * substitute(expr.rhs, lets);
}

template<expression Lhs, expression Rhs>
[[nodiscard]] constexpr auto operator*(const side<Lhs> &lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{multiply{lhs.expr, rhs.expr}};
}

template<expression Lhs, typename Rhs>
  requires std::is_arithmetic_v<Rhs>
[[nodiscard]] constexpr auto operator*(const side<Lhs> &lhs, Rhs rhs) noexcept(release) {
  return side{multiply{lhs.expr, rhs}};
}

template<typename Lhs, expression Rhs>
  requires std::is_arithmetic_v<Lhs>
[[nodiscard]] constexpr auto operator*(Lhs lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{multiply{lhs, rhs.expr}};
}

} // namespace upd::algebra
