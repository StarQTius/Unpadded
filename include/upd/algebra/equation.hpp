#pragma once

#include "../constexpr.hpp"
#include "../is_convertible_to_instance_of.hpp"
#include "../record/lite_record.hpp"
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
};

} // namespace upd::algebra
