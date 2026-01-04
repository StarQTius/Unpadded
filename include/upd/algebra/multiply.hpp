#pragma once

#include <format>
#include <type_traits>

#include "../record/record_like.hpp"
#include "../static_assert.hpp"
#include "../upd.hpp"
#include "concepts.hpp"
#include "side.hpp"

namespace upd::algebra {

template<expression, expression>
struct divide;

template<expression Lhs, expression Rhs>
struct multiply {
  Lhs lhs;
  Rhs rhs;

  [[nodiscard]] constexpr auto operator==(const multiply &) const noexcept(release) -> bool = default;
};

template<expression Lhs, expression Rhs, record_like Lets>
[[nodiscard]] constexpr auto substitute(const multiply<Lhs, Rhs> &expr, const Lets &lets) noexcept(release) {
  return substitute(expr.lhs, lets) * substitute(expr.rhs, lets);
}

template<expression Lhs, expression Rhs, auto Varname>
struct depends_on<multiply<Lhs, Rhs>, Varname> {
  constexpr static auto value = depends_on_v<Lhs, Varname> || depends_on_v<Rhs, Varname>;
};

template<auto Varname, expression Expr, expression Lhs, expression Rhs>
  requires balanceable<Lhs, Varname> || balanceable<Rhs, Varname>
[[nodiscard]] constexpr auto balance_on(const Expr &base, const multiply<Lhs, Rhs> &expr) noexcept(release) {
  UPD_STATIC_ASSERT((!depends_on_v<decltype(expr.lhs), Varname> || !depends_on_v<decltype(expr.rhs), Varname>),
                    "{} and {} both depend on {}; Only one operand should depend on {}",
                    expr.lhs,
                    expr.rhs,
                    Varname,
                    Varname);
  UPD_STATIC_ASSERT((depends_on_v<decltype(expr.lhs), Varname> || depends_on_v<decltype(expr.rhs), Varname>),
                    "Neither {} and {} depends on {}; At least one of them should depend on {}",
                    expr.lhs,
                    expr.rhs,
                    Varname,
                    Varname);

  if constexpr (depends_on_v<decltype(expr.lhs), Varname>) {
    return balance_on<Varname>(divide{base, expr.rhs}, expr.lhs);
  } else {
    return balance_on<Varname>(divide{base, expr.lhs}, expr.rhs);
  }
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

template<expression Lhs, expression Rhs>
struct divide {
  Lhs lhs;
  Rhs rhs;

  [[nodiscard]] constexpr auto operator==(const divide &) const noexcept(release) -> bool = default;
};

template<expression Lhs, expression Rhs, record_like Lets>
[[nodiscard]] constexpr auto substitute(const divide<Lhs, Rhs> &expr, const Lets &lets) noexcept(release) {
  return substitute(expr.lhs, lets) / substitute(expr.rhs, lets);
}

template<expression Lhs, expression Rhs, auto Varname>
struct depends_on<divide<Lhs, Rhs>, Varname> {
  constexpr static auto value = depends_on_v<Lhs, Varname> || depends_on_v<Rhs, Varname>;
};

template<auto Varname, expression Expr, expression Lhs, expression Rhs>
  requires balanceable<Lhs, Varname> || balanceable<Rhs, Varname>
[[nodiscard]] constexpr auto balance_on(const Expr &base, const divide<Lhs, Rhs> &expr) noexcept(release) {
  UPD_STATIC_ASSERT((!depends_on_v<decltype(expr.lhs), Varname> || !depends_on_v<decltype(expr.rhs), Varname>),
                    "{} and {} both depend on {}; Only one operand should depend on {}",
                    expr.lhs,
                    expr.rhs,
                    Varname,
                    Varname);
  UPD_STATIC_ASSERT((depends_on_v<decltype(expr.lhs), Varname> || depends_on_v<decltype(expr.rhs), Varname>),
                    "Neither {} and {} depends on {}; At least one of them should depend on {}",
                    expr.lhs,
                    expr.rhs,
                    Varname,
                    Varname);

  if constexpr (depends_on_v<decltype(expr.lhs), Varname>) {
    return balance_on<Varname>(multiply{base, expr.rhs}, expr.lhs);
  } else {
    return balance_on<Varname>(divide{expr.lhs, base}, expr.rhs);
  }
}

template<expression Lhs, expression Rhs>
[[nodiscard]] constexpr auto operator/(const side<Lhs> &lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{divide{lhs.expr, rhs.expr}};
}

template<expression Lhs, typename Rhs>
  requires std::is_arithmetic_v<Rhs>
[[nodiscard]] constexpr auto operator/(const side<Lhs> &lhs, Rhs rhs) noexcept(release) {
  return side{divide{lhs.expr, rhs}};
}

template<typename Lhs, expression Rhs>
  requires std::is_arithmetic_v<Lhs>
[[nodiscard]] constexpr auto operator/(Lhs lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{divide{lhs, rhs.expr}};
}

} // namespace upd::algebra

template<upd::algebra::expression Lhs, upd::algebra::expression Rhs>
struct std::formatter<upd::algebra::multiply<Lhs, Rhs>> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const upd::algebra::multiply<Lhs, Rhs> &eq, std::format_context &ctx) {
    auto it = ctx.out();

    it = std::format_to(it, "{} * {}", eq.lhs, eq.rhs);

    ctx.advance_to(it);
    return it;
  }
};

template<upd::algebra::expression Lhs, upd::algebra::expression Rhs>
struct std::formatter<upd::algebra::divide<Lhs, Rhs>> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const upd::algebra::divide<Lhs, Rhs> &eq, std::format_context &ctx) {
    auto it = ctx.out();

    it = std::format_to(it, "{} / {}", eq.lhs, eq.rhs);

    ctx.advance_to(it);
    return it;
  }
};
