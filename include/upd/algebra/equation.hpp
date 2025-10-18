#pragma once

#include "../constexpr.hpp"
#include "../is_convertible_to_instance_of.hpp"
#include "../record/lite_record.hpp"
#include "../static_assert.hpp"
#include "../upd.hpp"
#include "concepts.hpp"
#include "let.hpp"
#include "variable.hpp"

namespace upd::algebra {

template<expression Lhs, expression Rhs>
struct equation {
  Lhs lhs;
  Rhs rhs;

  template<auto... Varnames, typename... Vals>
    requires(sizeof...(Varnames) == sizeof...(Vals))
  [[nodiscard]] constexpr auto substitute(const let<Varnames, Vals> &...lets) noexcept(release) {
    using upd::algebra::substitute;

    auto letrec = lite_record{lite_record_node{upd::expr<lets.var.name>, lets.val}...};
    return upd::algebra::equation{
        .lhs = substitute(lhs, letrec),
        .rhs = substitute(rhs, letrec),
    };
  }

  template<typename... Ts>
    requires(is_convertible_to_instance_of<Ts, let>() && ...)
  [[nodiscard]] constexpr auto substitute(const Ts &...xs) noexcept(release) {
    return substitute(let{xs}...);
  }

  template<auto Varname>
    requires balanceable<Lhs, Varname> || balanceable<Rhs, Varname>
  [[nodiscard]] constexpr auto isolate(side<variable<Varname>> var) noexcept(release) {
    UPD_STATIC_ASSERT((!depends_on_v<decltype(lhs), Varname> || !depends_on_v<decltype(rhs), Varname>),
                      "{} and {} both depend on {}; Only one operand should depend on {}",
                      lhs,
                      rhs,
                      var,
                      var);
    UPD_STATIC_ASSERT((depends_on_v<decltype(lhs), Varname> || depends_on_v<decltype(rhs), Varname>),
                      "Neither {} and {} depends on {}; At least one of them should depend on {}",
                      lhs,
                      rhs,
                      var,
                      var);

    if constexpr (depends_on_v<decltype(lhs), Varname>) {
      return upd::algebra::equation{
          .lhs = var.expr,
          .rhs = balance_on<Varname>(rhs, lhs),
      };
    } else {
      return upd::algebra::equation{
          .lhs = var.expr,
          .rhs = balance_on<Varname>(lhs, rhs),
      };
    }
  }

  [[nodiscard]] constexpr auto operator==(const equation &) const noexcept(release) -> bool = default;
};

} // namespace upd::algebra
