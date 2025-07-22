#pragma once

#include <cstddef>

#include "constexpr.hpp"
#include "detail/ascii_to_integer.hpp"
#include "detail/static_storage.hpp" // IWYU pragma: keep

namespace upd::literals {

template<char... Cs>
[[nodiscard]] constexpr auto operator""_ic() noexcept {
  constexpr auto &characters = detail::static_storage<Cs...>;
  constexpr auto begin = characters.begin();
  constexpr auto end = characters.end();
  constexpr auto retval = detail::ascii_to_integer(begin, end);

  return expr<retval>;
}

template<char... Cs>
[[nodiscard]] constexpr auto operator""_i() noexcept {
  constexpr auto &characters = detail::static_storage<Cs...>;
  constexpr auto begin = characters.begin();
  constexpr auto end = characters.end();
  constexpr auto value = detail::ascii_to_integer(begin, end);
  constexpr auto retval = static_cast<std::size_t>(value);

  static_assert(value == retval, "Literal value is too large for `std::size_t`");

  return expr<retval>;
}

} // namespace upd::literals
