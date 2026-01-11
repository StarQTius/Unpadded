#pragma once

#include <format>
#include <tuple>

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

template<auto Varname>
[[nodiscard]] constexpr auto dependencies(variable<Varname> var) noexcept(release) {
  return std::tuple{var};
}

} // namespace upd::algebra

template<auto Name>
struct std::formatter<upd::algebra::variable<Name>> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(upd::algebra::variable<Name>, std::format_context &ctx) {
    auto it = ctx.out();

    it = std::format_to(it, "{}", Name);

    ctx.advance_to(it);
    return it;
  }
};
