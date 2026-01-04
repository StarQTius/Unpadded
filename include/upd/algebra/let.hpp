#pragma once

#include <functional>
#include <type_traits>

#include "../concept/invocable.hpp"
#include "../upd.hpp"
#include "concepts.hpp"
#include "variable.hpp"

namespace upd::algebra {

template<expression Lhs, expression Rhs>
struct equation;

template<expression Expr>
struct side;

template<auto Varname, typename Val>
  requires std::is_scalar_v<Val>
struct let {
  explicit constexpr let(variable<Varname> var, Val val) : var{var}, val{val} {}

  explicit constexpr let(const side<variable<Varname>> &s, Val val) : var{s.expr}, val{val} {}

  explicit constexpr let(const equation<variable<Varname>, Val> &eq) : var{eq.lhs}, val{eq.rhs} {}

  explicit constexpr let(const equation<variable<Varname>, std::reference_wrapper<Val>> &eq)
      : var{eq.lhs}, val{eq.rhs} {}

  template<invocable F>
  explicit constexpr let(const equation<variable<Varname>, F> eq) : var{eq.lhs}, val{UPD_INVOKE(eq.rhs)} {}

  variable<Varname> var;
  Val val;
};

template<auto Varname, invocable F>
explicit let(equation<variable<Varname>, F>) -> let<Varname, std::invoke_result_t<F>>;

} // namespace upd::algebra
