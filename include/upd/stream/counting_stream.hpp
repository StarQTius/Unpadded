#pragma once

#include <cstddef>

#include "../upd.hpp"
#include "stream_interface.hpp"

namespace upd {

class counting_stream : public stream_interface {
public:
  counting_stream() = default;

  [[nodiscard]] auto read(std::size_t bitcount, char *) noexcept(release)
      -> lite_error_t override {
    m_read += bitcount;
    return lite_error::none;
  }

  [[nodiscard]] auto write(const char *, std::size_t bitsize) noexcept(release)
      -> lite_error_t override {
    m_written += bitsize;
    return lite_error::none;
  }

  [[nodiscard]] constexpr auto read() const noexcept(release) -> std::size_t {
    return m_read;
  }

  [[nodiscard]] constexpr auto
  written() const noexcept(release) -> std::size_t {
    return m_written;
  }

private:
  std::size_t m_read;
  std::size_t m_written;
};

} // namespace upd
