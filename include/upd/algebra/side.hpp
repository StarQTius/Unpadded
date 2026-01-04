#pragma once

#include <functional>
#include <type_traits>

#include "../concept/invocable.hpp"
#include "../constexpr.hpp"
#include "../is_convertible_to_instance_of.hpp"
#include "../record/lite_record.hpp"
#include "../record/name.hpp"
#include "../static_assert.hpp"
#include "../upd.hpp"
#include "concepts.hpp"
#include "equation.hpp"
#include "let.hpp"
#include "variable.hpp"

namespace upd::algebra {

template<expression Expr>
struct side {
  Expr expr;

  template<auto... Varnames, typename... Vals>
    requires(sizeof...(Varnames) == sizeof...(Vals))
  [[nodiscard]] constexpr auto substitute(const let<Varnames, Vals> &...lets) const noexcept(release) {
    using ::upd::algebra::substitute;
    return substitute(expr, lite_record{lite_record_node{upd::expr<lets.var.name>, lets.val}...});
  }

  template<typename... Ts>
    requires(is_convertible_to_instance_of<Ts, let>() && ...)
  [[nodiscard]] constexpr auto substitute(const Ts &...xs) noexcept(release) {
    return substitute(let{xs}...);
  }

  template<auto... Varnames, typename... Vals>
    requires(sizeof...(Varnames) == sizeof...(Vals))
  [[nodiscard]] constexpr auto calculate(const let<Varnames, Vals> &...lets) const noexcept(release) {
    using ::upd::algebra::substitute;
    auto retval = substitute(expr, lite_record{lite_record_node{upd::expr<lets.var.name>, lets.val}...});

    using retval_type = decltype(retval);
    UPD_STATIC_ASSERT(std::is_scalar_v<retval_type>,
                      "Substitution resulted in value of type `{}` which is not an arithmetic type",
                      typeid(retval_type));

    return retval;
  }

  template<typename... Ts>
    requires(sizeof...(Ts) > 0 && (is_convertible_to_instance_of<Ts, let>() && ...))
  [[nodiscard]] constexpr auto calculate(const Ts &...xs) const noexcept(release) {
    return calculate(let{xs}...);
  }

  template<typename Self, expression E>
  [[nodiscard]] constexpr auto operator=(this Self &&self, const side<E> &s) noexcept(release) {
    return equation{
        .lhs = UPD_FWD(self).expr,
        .rhs = UPD_FWD(s).expr,
    };
  }

  template<typename Self, typename T>
    requires std::is_scalar_v<T>
  [[nodiscard]] constexpr auto operator=(this Self &&self, T value) noexcept(release) {
    return equation{
        .lhs = UPD_FWD(self).expr,
        .rhs = value,
    };
  }

  template<typename Self, typename T>
    requires std::is_scalar_v<T>
  [[nodiscard]] constexpr auto operator=(this Self &&self, std::reference_wrapper<T> ref) noexcept(release) {
    return equation{
        .lhs = UPD_FWD(self).expr,
        .rhs = ref,
    };
  }

  template<typename Self, invocable F>
  [[nodiscard]] constexpr auto operator=(this Self &&self, F &&f) noexcept(release) {
    return equation{
        .lhs = UPD_FWD(self).expr,
        .rhs = UPD_FWD(f),
    };
  }

  [[nodiscard]] constexpr auto operator==(const side &) const noexcept(release) -> bool = default;
};

} // namespace upd::algebra

namespace upd::algebra::literals {

template<name Varname>
[[nodiscard]] constexpr auto operator""_var() noexcept(release) {
  return side{variable<Varname>{}};
}

} // namespace upd::algebra::literals
