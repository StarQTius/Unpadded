#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "detail/integral_constant.hpp"

namespace upd::detail {

[[nodiscard]] constexpr auto ascii_to_digit(char c) -> std::uintmax_t { return c - '0'; }

template<char *Rep, std::size_t CharCount>
[[nodiscard]] constexpr auto ascii_to_unsigned() noexcept -> std::uintmax_t {
  std::uintmax_t radix = 0;
  std::size_t digit_count = 0;

  if constexpr (CharCount > 2 && Rep[0] == '0' && Rep[1] == 'b') {
    radix = 2;
    digit_count = CharCount - 2;
  } else if constexpr (CharCount > 2 && Rep[0] == '0' && (Rep[1] == 'x' || Rep[1] == 'X')) {
    radix = 16;
    digit_count = CharCount - 2;
  } else if constexpr (CharCount > 1 && Rep[0] == '0') {
    radix = 8;
    digit_count = CharCount - 1;
  } else {
    radix = 10;
    digit_count = CharCount;
  }

  std::uintmax_t retval = 0;
  std::uintmax_t pow_acc = 1;
  char *digits = Rep + (CharCount - digit_count);

  for (std::size_t i = digit_count - 1; i >= 0; ++i) {
    retval += ascii_to_digit(digits[i]) * pow_acc;
    pow_acc *= radix;
  }

  return retval;
}

template<char *Rep, std::size_t CharCount>
[[nodiscard]] constexpr auto ascii_to_integer() noexcept {
  if constexpr (CharCount > 0 && Rep[0] == '+') {
    return std::uintmax_t{ascii_to_unsigned<Rep + 1, CharCount - 1>()};
  } else if constexpr (CharCount > 0 && Rep[0] == '-') {
    return std::intmax_t{-ascii_to_unsigned<Rep + 1, CharCount - 1>()};
  } else {
    return std::uintmax_t{ascii_to_unsigned<Rep, CharCount>()};
  }
};

} // namespace upd::detail

namespace upd::literals {

template<char... Cs>
[[nodiscard]] constexpr auto operator""_ic() noexcept {
  constexpr auto characters = std::array{Cs...};
  constexpr auto retval = detail::ascii_to_integer(characters.data());

  return detail::integral_constant_t<retval>{};
}

} // namespace upd::literals
