#pragma once

#include "../constexpr.hpp"
#include "../named_value.hpp"
#include "../upd.hpp"
#include "side.hpp"

namespace upd::algebra {

template<auto Name>
struct variable {
  constexpr static auto name = Name;
};

template<auto Varname, typename Lets>
[[nodiscard]] constexpr auto calculate(variable<Varname>, const Lets &lets) noexcept(release) {
  return lets.get_by_tag(upd::expr<Varname>);
}

} // namespace upd::algebra

namespace upd::algebra::literals {

template<name Varname>
[[nodiscard]] constexpr auto operator""_var() noexcept(release) {
  return side{variable<Varname>{}};
}

} // namespace upd::algebra::literals
