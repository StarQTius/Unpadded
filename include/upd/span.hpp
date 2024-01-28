#pragma once

#include <cstddef>

template<typename T>
class span {
public:
  constexpr explicit span(T *ptr, std::size_t size) noexcept : m_ptr{ptr}, m_size{size} {}

  [[nodiscard]] constexpr auto size() const noexcept -> std::size_t { return m_size; }

  [[nodiscard]] constexpr auto data() const noexcept -> T * { return m_ptr; }

  [[nodiscard]] constexpr auto begin() const noexcept -> T * { return m_ptr; }

  [[nodiscard]] constexpr auto end() const noexcept -> T * { return m_ptr + m_size; }

private:
  T *m_ptr;
  std::size_t m_size;
};
