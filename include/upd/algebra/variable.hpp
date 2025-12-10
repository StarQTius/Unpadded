#pragma once

#include "../get.hpp"
#include "../record/lite_record.hpp"
#include "../record/record_like.hpp"
#include "../upd.hpp"
#include "concepts.hpp"

namespace upd::algebra {

template<auto Name>
struct variable {
  constexpr static auto name = Name;

  [[nodiscard]] constexpr auto operator==(variable<Name>) const noexcept(release) -> bool { return true; }

  template<auto VN>
  [[nodiscard]] constexpr auto operator==(variable<VN>) const noexcept(release) -> bool {
    return false;
  }
};

template<auto Varname, record_like Lets>
[[nodiscard]] constexpr auto substitute(variable<Varname>, const Lets &lets) noexcept(release) {
  if constexpr (has_tag<Varname>(lets)) {
    return get<Varname>(lets);
  } else {
    return variable<Varname>{};
  }
}

template<auto Varname, auto OtherVarname>
struct depends_on<variable<OtherVarname>, Varname> {
  constexpr static auto value = false;
};

template<auto Varname>
struct depends_on<variable<Varname>, Varname> {
  constexpr static auto value = true;
};

template<auto Varname, expression Expr>
[[nodiscard]] constexpr auto balance_on(const Expr &base, variable<Varname>) noexcept(release) -> Expr {
  return base;
}

} // namespace upd::algebra
