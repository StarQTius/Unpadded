#pragma once

#include <type_traits>

#include "../constexpr.hpp"
#include "../lite_record.hpp"
#include "../upd.hpp"

namespace upd::algebra {

template<typename T, typename Lets>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr auto calculate(T value, const Lets &) noexcept(release) {
  return value;
}

template<typename Var, typename Val>
  requires std::is_arithmetic_v<Val>
struct let {
  Var var;
  Val val;
};

template<typename Expr>
struct side {
  Expr expr;

  template<typename... Vars, typename... Vals>
    requires(sizeof...(Vars) == sizeof...(Vals))
  [[nodiscard]] constexpr auto calculate(const let<Vars, Vals> &...lets) const noexcept(release) {
    using ::upd::algebra::calculate;
    return calculate(expr, lite_record{lite_record_node{upd::expr<lets.var.expr.name>, lets.val}...});
  }
};

} // namespace upd::algebra
