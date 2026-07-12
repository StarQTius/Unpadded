#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iterator>

#include "../error.hpp"
#include "stream_interface.hpp"
#include <limits.h>

namespace upd {

template<std::input_iterator InputIt, typename OutputIt>
  requires std::output_iterator<OutputIt, char>
           && std::convertible_to<std::iter_value_t<InputIt>, char>
class iterator_stream : public stream_interface {
public:
  constexpr explicit iterator_stream(InputIt in, OutputIt out)
      : m_in{in}, m_out{out} {}

  auto read(std::size_t bitcount, char *dest) -> lite_error_t override {
    if (bitcount % CHAR_BIT != 0) {
      return lite_error::not_a_multiple;
    }

    std::copy_n(m_in, bitcount / CHAR_BIT, dest);
    std::advance(m_in, bitcount / CHAR_BIT);
    return lite_error::none;
  }

  auto write(const char *src, std::size_t bitsize) -> lite_error_t override {
    if (bitsize % CHAR_BIT != 0) {
      return lite_error::not_a_multiple;
    }

    m_out = std::copy_n(src, bitsize / CHAR_BIT, m_out);
    return lite_error::none;
  }

private:
  InputIt m_in;
  OutputIt m_out;
};

} // namespace upd
