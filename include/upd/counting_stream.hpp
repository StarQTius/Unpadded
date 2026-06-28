#pragma once

#include <cstddef>

#include "stream_interface.hpp"
#include "upd.hpp"

namespace upd {

class counting_stream : public stream_interface {
public:
  counting_stream() = default;

  [[nodiscard]] auto read(std::size_t count, word_t *) noexcept(release) -> stream_error_t override {
    m_read += count;
    return 0;
  }

  [[nodiscard]] auto write(const word_t *, std::size_t size) noexcept(release) -> stream_error_t override {
    m_written += size;
    return 0;
  }

  [[nodiscard]] constexpr auto read() const noexcept(release) -> std::size_t { return m_read; }

  [[nodiscard]] constexpr auto written() const noexcept(release) -> std::size_t { return m_written; }

private:
  std::size_t m_read;
  std::size_t m_written;
};

} // namespace upd
