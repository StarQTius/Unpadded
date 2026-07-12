#pragma once

#include <algorithm>
#include <climits>
#include <cstddef>
#include <iosfwd>
#include <istream>
#include <ranges>

#include "../error.hpp"
#include "stream_interface.hpp"

namespace upd {

class standard_stream : public stream_interface {
public:
  constexpr explicit standard_stream(std::istream *in,
                                     std::ostream *out,
                                     const char *sep)
      : m_in{in}, m_out{out}, m_sep{sep} {}

  auto read(std::size_t bitcount, char *dest) -> lite_error_t override {
    if (bitcount % CHAR_BIT != 0) {
      return lite_error::not_a_multiple;
    }

    std::generate_n(dest, bitcount / CHAR_BIT, [this] {
      auto w = char{0};
      *m_in >> w;
      return w;
    });

    return lite_error::none;
  }

  auto write(const char *src, std::size_t bitsize) -> lite_error_t override {
    namespace stdr = std::ranges;

    if (bitsize % CHAR_BIT != 0) {
      return lite_error::not_a_multiple;
    }

    for (auto w : stdr::subrange{src, src + bitsize / CHAR_BIT}) {
      *m_out << w << m_sep;
    }

    return lite_error::none;
  }

private:
  std::istream *m_in;
  std::ostream *m_out;
  const char *m_sep;
};

} // namespace upd
