#pragma once

#include "detail/ascii_to_integer.hpp"
#include "detail/integral_constant.hpp"
#include "detail/static_storage.hpp" // IWYU pragma: keep

namespace upd::literals {

template<char... Cs>
[[nodiscard]] constexpr auto operator""_ic() noexcept {
  constexpr auto &characters = detail::static_storage<Cs...>;
  constexpr auto begin = characters.begin();
  constexpr auto end = characters.end();
  constexpr auto retval = detail::ascii_to_integer(begin, end);

  return detail::integral_constant_t<retval>{};
}

} // namespace upd::literals
