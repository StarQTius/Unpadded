#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <ranges>
#include <type_traits>

#include "../error.hpp"
#include <limits.h>

namespace upd {

using uword_t = std::uintmax_t;
using word_t = std::make_signed_t<uword_t>;

constexpr auto max_bitsize = std::numeric_limits<uword_t>::digits;

struct stream_interface {
  [[nodiscard]] virtual auto read(std::size_t, char *) -> lite_error_t = 0;

  [[nodiscard]] virtual auto write(const char *, std::size_t) -> lite_error_t = 0;

  [[nodiscard]] virtual auto read_signed(std::size_t bitsize, uword_t *dest) -> lite_error_t {
    using namespace std::views;

    if (bitsize > max_bitsize) {
      return lite_error::buffer_too_small;
    }

    auto retval = uword_t{0};
    auto buf = std::array<char, max_bitsize / CHAR_BIT>{};
    if (auto err = read(bitsize, buf.data()); err) {
      return err;
    }

    auto bytecount = (bitsize / CHAR_BIT) + (bitsize % CHAR_BIT > 0);
    for (unsigned char b : buf | take(bytecount) | reverse) {
      retval <<= CHAR_BIT;
      retval |= b;
    }

    auto mask = uword_t{1} << (bitsize - 1);
    *dest = (retval ^ mask) - mask;
    return lite_error::none;
  }

  [[nodiscard]] virtual auto write_signed(uword_t src, std::size_t bitcount) -> lite_error_t {
    using namespace std::views;

    if (bitcount > max_bitsize) {
      return lite_error::buffer_too_small;
    }

    auto buf = std::array<char, max_bitsize / CHAR_BIT>{};
    auto bytecount = (bitcount / CHAR_BIT) + (bitcount % CHAR_BIT > 0);
    for (auto &b : buf | take(bytecount)) {
      b = src & UCHAR_MAX;
      src >>= CHAR_BIT;
    }

    if (auto err = write(buf.data(), bitcount); err) {
      return err;
    }

    return lite_error::none;
  }

  [[nodiscard]] virtual auto read_unsigned(std::size_t bitsize, uword_t *dest) -> lite_error_t {
    using namespace std::views;

    if (bitsize > max_bitsize) {
      return lite_error::buffer_too_small;
    }

    auto retval = uword_t{0};
    auto buf = std::array<char, max_bitsize / CHAR_BIT>{};
    if (auto err = read(bitsize, buf.data()); err) {
      return err;
    }

    auto bytecount = (bitsize / CHAR_BIT) + (bitsize % CHAR_BIT > 0);
    for (unsigned char b : buf | take(bytecount) | reverse) {
      retval <<= CHAR_BIT;
      retval |= b;
    }

    *dest = retval;
    return lite_error::none;
  }

  [[nodiscard]] virtual auto write_unsigned(uword_t src, std::size_t bitcount) -> lite_error_t {
    using namespace std::views;

    if (bitcount > max_bitsize) {
      return lite_error::buffer_too_small;
    }

    auto buf = std::array<char, max_bitsize / CHAR_BIT>{};
    auto bytecount = (bitcount / CHAR_BIT) + (bitcount % CHAR_BIT > 0);
    for (auto &b : buf | take(bytecount)) {
      b = src & UCHAR_MAX;
      src >>= CHAR_BIT;
    }

    if (auto err = write(buf.data(), bitcount); err) {
      return err;
    }

    return lite_error::none;
  }
};

} // namespace upd
