#pragma once

#include <format>
#include <tuple>
#include <type_traits>

#include "../record/record_like.hpp"
#include "../tuple/concat.hpp"
#include "../tuple/to.hpp"
#include "../upd.hpp"
#include "../utility/static_assert.hpp"
#include "concepts.hpp"
#include "side.hpp"

namespace upd::algebra {

template<expression Lhs, expression Rhs>
struct add {
  Lhs lhs;
  Rhs rhs;

  [[nodiscard]] constexpr auto operator==(const add &) const noexcept(release) -> bool = default;
};

template<expression Lhs, expression Rhs, record_like Lets>
[[nodiscard]] constexpr auto substitute(const add<Lhs, Rhs> &expr, const Lets &lets) noexcept(release) {
  return substitute(expr.lhs, lets) + substitute(expr.rhs, lets);
}

template<expression Lhs, expression Rhs, auto Varname>
struct depends_on<add<Lhs, Rhs>, Varname> {
  constexpr static auto value = depends_on_v<Lhs, Varname> || depends_on_v<Rhs, Varname>;
};

template<auto Varname, expression Expr, expression Lhs, expression Rhs>
  requires balanceable<Lhs, Varname> || balanceable<Rhs, Varname>
[[nodiscard]] constexpr auto balance_on(const Expr &base, const add<Lhs, Rhs> &expr) noexcept(release) {
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
    return balance_on<Varname>(base - expr.rhs, expr.lhs);
  } else {
    return balance_on<Varname>(base - expr.lhs, expr.rhs);
  }
}

template<expression Lhs, expression Rhs>
[[nodiscard]] constexpr auto dependencies(const add<Lhs, Rhs> &expr) noexcept(release) {
  namespace updv = upd::tuple_views;
  return updv::concat(dependencies(expr.lhs), dependencies(expr.rhs)) | updv::to<std::tuple>;
}

template<expression Lhs, expression Rhs>
[[nodiscard]] constexpr auto operator+(const side<Lhs> &lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{add{lhs.expr, rhs.expr}};
}

template<expression Lhs, typename Rhs>
  requires std::is_arithmetic_v<Rhs>
[[nodiscard]] constexpr auto operator+(const side<Lhs> &lhs, Rhs rhs) noexcept(release) {
  return side{add{lhs.expr, rhs}};
}

template<typename Lhs, expression Rhs>
  requires std::is_arithmetic_v<Lhs>
[[nodiscard]] constexpr auto operator+(Lhs lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{add{lhs, rhs.expr}};
}

template<expression Lhs, expression Rhs>
[[nodiscard]] constexpr auto operator+(const Lhs &lhs, const Rhs &rhs) noexcept(release) {
  return add{lhs, rhs};
}

template<expression Lhs, expression Rhs>
struct substract {
  Lhs lhs;
  Rhs rhs;

  [[nodiscard]] constexpr auto operator==(const substract &) const noexcept(release) -> bool = default;
};

template<expression Lhs, expression Rhs, record_like Lets>
[[nodiscard]] constexpr auto substitute(const substract<Lhs, Rhs> &expr, const Lets &lets) noexcept(release) {
  return substitute(expr.lhs, lets) - substitute(expr.rhs, lets);
}

template<expression Lhs, expression Rhs, auto Varname>
struct depends_on<substract<Lhs, Rhs>, Varname> {
  constexpr static auto value = depends_on_v<Lhs, Varname> || depends_on_v<Rhs, Varname>;
};

template<auto Varname, expression Expr, expression Lhs, expression Rhs>
  requires balanceable<Lhs, Varname> || balanceable<Rhs, Varname>
[[nodiscard]] constexpr auto balance_on(const Expr &base, const substract<Lhs, Rhs> &expr) noexcept(release) {
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
    return balance_on<Varname>(base + expr.rhs, expr.lhs);
  } else {
    return balance_on<Varname>(expr.lhs - base, expr.rhs);
  }
}

template<expression Lhs, expression Rhs>
[[nodiscard]] constexpr auto dependencies(const substract<Lhs, Rhs> &expr) noexcept(release) {
  namespace updv = upd::tuple_views;
  return updv::concat(dependencies(expr.lhs), dependencies(expr.rhs)) | updv::to<std::tuple>;
}

template<expression Lhs, expression Rhs>
[[nodiscard]] constexpr auto operator-(const side<Lhs> &lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{substract{lhs.expr, rhs.expr}};
}

template<expression Lhs, typename Rhs>
  requires std::is_arithmetic_v<Rhs>
[[nodiscard]] constexpr auto operator-(const side<Lhs> &lhs, Rhs rhs) noexcept(release) {
  return side{substract{lhs.expr, rhs}};
}

template<typename Lhs, expression Rhs>
  requires std::is_arithmetic_v<Lhs>
[[nodiscard]] constexpr auto operator-(Lhs lhs, const side<Rhs> &rhs) noexcept(release) {
  return side{substract{lhs, rhs.expr}};
}

template<expression Lhs, expression Rhs>
[[nodiscard]] constexpr auto operator-(const Lhs &lhs, const Rhs &rhs) noexcept(release) {
  return substract{lhs, rhs};
}

} // namespace upd::algebra

template<upd::algebra::expression Lhs, upd::algebra::expression Rhs>
struct std::formatter<upd::algebra::add<Lhs, Rhs>> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const upd::algebra::add<Lhs, Rhs> &eq, std::format_context &ctx) {
    auto it = ctx.out();

    it = std::format_to(it, "({} + {})", eq.lhs, eq.rhs);

    ctx.advance_to(it);
    return it;
  }
};

template<upd::algebra::expression Lhs, upd::algebra::expression Rhs>
struct std::formatter<upd::algebra::substract<Lhs, Rhs>> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const upd::algebra::substract<Lhs, Rhs> &eq, std::format_context &ctx) {
    auto it = ctx.out();

    it = std::format_to(it, "({} - {})", eq.lhs, eq.rhs);

    ctx.advance_to(it);
    return it;
  }
};
