#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <istream>
#include <iterator>
#include <ostream>

namespace upd {

using word_t = std::uintmax_t;
using stream_error_t = std::intptr_t;

struct stream_interface {
  [[nodiscard]] virtual auto read(std::size_t, word_t *) -> stream_error_t = 0;
  [[nodiscard]] virtual auto write(const word_t *, std::size_t) -> stream_error_t = 0;
};

template<std::input_iterator InputIt, typename OutputIt>
  requires std::output_iterator<OutputIt, word_t> && std::convertible_to<std::iter_value_t<InputIt>, word_t>
class iterator_stream : public stream_interface {
public:
  constexpr explicit iterator_stream(InputIt in, OutputIt out) : m_in{in}, m_out{out} {}

  auto read(std::size_t count, word_t *dest) -> stream_error_t override {
    std::copy_n(m_in, count, dest);
    std::advance(m_in, count);
    return 0;
  }

  auto write(const word_t *src, std::size_t size) -> stream_error_t override {
    m_out = std::copy_n(src, size, m_out);
    return 0;
  }

private:
  InputIt m_in;
  OutputIt m_out;
};

class standard_stream : public stream_interface {
public:
  constexpr explicit standard_stream(std::istream *in, std::ostream *out, const char *sep)
      : m_in{in}, m_out{out}, m_sep{sep} {}

  auto read(std::size_t count, word_t *dest) -> stream_error_t override {
    UPD_ASSERT(m_in);

    std::generate_n(dest, count, [this] {
      auto w = word_t{};
      *m_in >> w;
      return w;
    });

    return 0;
  }

  auto write(const word_t *src, std::size_t size) -> stream_error_t override {
    namespace stdr = std::ranges;

    UPD_ASSERT(m_out);

    for (auto w : stdr::subrange{src, src + size}) {
      *m_out << w << m_sep;
    }

    return 0;
  }

private:
  std::istream *m_in;
  std::ostream *m_out;
  const char *m_sep;
};

} // namespace upd
