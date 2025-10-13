#pragma once

#include <type_traits>

#include "concepts.hpp"
#include "variable.hpp"

namespace upd::algebra {

template<expression Lhs, expression Rhs>
struct equation;

template<expression Expr>
struct side;

template<auto Varname, typename Val>
  requires std::is_arithmetic_v<Val>
struct let {
  explicit constexpr let(variable<Varname> var, Val val) : var{var}, val{val} {}

  explicit constexpr let(const side<variable<Varname>> &s, Val val) : var{s.expr}, val{val} {}

  explicit constexpr let(const equation<variable<Varname>, Val> &eq) : var{eq.lhs}, val{eq.rhs} {}

  variable<Varname> var;
  Val val;
};

} // namespace upd::algebra
