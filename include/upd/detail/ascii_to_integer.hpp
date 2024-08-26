#pragma once

#include <cstdint>
#include <iterator>
#include <utility>

#include "fail_unless_discarded.hpp"
#include "range.hpp"

namespace upd::detail {

[[nodiscard]] constexpr auto ascii_to_digit(char c) noexcept -> std::uint8_t {
  if ('0' <= c && c <= '9') {
    return c - '0';
  }

  if ('a' <= c && c <= 'f') {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    return c - 'a' + 0xa;
  }

  if ('A' <= c && c <= 'F') {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    return c - 'A' + 0xa;
  }

  fail_unless_discarded("`c` is not a digit character");
}

template<typename It>
[[nodiscard]] constexpr auto ascii_to_integer(It begin, It end) -> std::intmax_t {
  auto range_size = std::distance(begin, end);
  auto first_char = range_size >= 1 ? *begin : 0;
  auto second_char = range_size >= 2 ? *std::next(begin) : 0;

  auto [radix, digit_begin] = [&] {
    if (first_char == '0' && second_char == 'b') {
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      return std::pair{2, std::next(begin, 2)};
    }

    if (first_char == '0' && (second_char == 'x' || second_char == 'X')) {
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      return std::pair{16, std::next(begin, 2)};
    }

    if (first_char == '0') {
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
      return std::pair{8, std::next(begin)};
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    return std::pair{10, begin};
  }();

  auto retval = std::intmax_t{0};
  auto pow_acc = std::intmax_t{1};
  auto rbegin = std::reverse_iterator{end};
  auto rend = std::reverse_iterator{digit_begin};

  for (auto c : range{rbegin, rend}) {
    if (c == '\'') {
      continue;
    }

    retval += ascii_to_digit(c) * pow_acc;
    pow_acc *= radix;
  }

  return retval;
}

} // namespace upd::detail
