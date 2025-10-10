#pragma once

#include <type_traits>

#include "../constexpr.hpp"
#include "../named_value.hpp"
#include "../record/concepts.hpp"
#include "../record/lite_record.hpp"
#include "../upd.hpp"
#include "concepts.hpp"

namespace upd::algebra {

template<auto Varname, typename Val>
  requires std::is_arithmetic_v<Val>
struct let;

template<expression Expr>
struct side {
  Expr expr;

  template<auto... Varnames, typename... Vals>
    requires(sizeof...(Varnames) == sizeof...(Vals))
  [[nodiscard]] constexpr auto calculate(const let<Varnames, Vals> &...lets) const noexcept(release) {
    using ::upd::algebra::calculate;
    return calculate(expr, lite_record{lite_record_node{upd::expr<lets.var.name>, lets.val}...});
  }
};

template<auto Name>
struct variable {
  constexpr static auto name = Name;
};

template<auto Varname, record_like Lets>
[[nodiscard]] constexpr auto calculate(variable<Varname>, const Lets &lets) noexcept(release) {
  return get<Varname>(lets);
}

template<auto Varname, typename Val>
  requires std::is_arithmetic_v<Val>
struct let {
  explicit constexpr let(variable<Varname> var, Val val) : var{var}, val{val} {}

  explicit constexpr let(side<variable<Varname>> s, Val val) : var{s.expr}, val{val} {}

  variable<Varname> var;
  Val val;
};

} // namespace upd::algebra

namespace upd::algebra::literals {

template<name Varname>
[[nodiscard]] constexpr auto operator""_var() noexcept(release) {
  return side{variable<Varname>{}};
}

} // namespace upd::algebra::literals
