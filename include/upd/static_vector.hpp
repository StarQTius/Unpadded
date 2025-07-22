#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <ranges>

namespace upd {

template<typename T, std::size_t Capacity>
class static_vector {
public:
  using value_type = T;

  constexpr static_vector() noexcept(release) : m_content{}, m_size{0} {}

  template<typename U, std::size_t N>
  constexpr static_vector(const std::array<U, N> &other) : static_vector{} {
    for (const auto &val : other) {
      push_back(val);
    }
  }

  [[nodiscard]] constexpr auto begin() noexcept(release) -> value_type * {
    return reinterpret_cast<value_type *>(m_content.begin());
  }

  [[nodiscard]] constexpr auto begin() const noexcept(release) -> const value_type * {
    return reinterpret_cast<const value_type *>(m_content.begin());
  }

  [[nodiscard]] constexpr auto end() noexcept(release) -> value_type * {
    return reinterpret_cast<value_type *>(m_content.begin()) + m_size;
  }

  [[nodiscard]] constexpr auto end() const noexcept(release) -> const value_type * {
    return reinterpret_cast<const value_type *>(m_content.begin()) + m_size;
  }

  template<typename U, std::size_t N>
  [[nodiscard]] constexpr auto operator==(const static_vector<U, N> &other) const -> bool {
    if (m_size != other.m_size) {
      return false;
    }

    return std::equal(m_content.begin(), m_content.begin() + m_size, other.m_content.begin());
  }

  constexpr void push_back(const T &value) {
    [[maybe_unused]] auto success = try_push_back(value);
    UPD_ASSERT(success);
  }

  constexpr void push_back(T &&value) {
    [[maybe_unused]] auto success = try_push_back(std::move(value));
    UPD_ASSERT(success);
  }

  [[nodiscard]] constexpr auto try_push_back(const T &value) -> bool {
    if (m_content.size() == m_size) {
      return false;
    }

    m_content[m_size].value = value;
    ++m_size;

    return true;
  }

  [[nodiscard]] constexpr auto try_push_back(T &&value) -> bool {
    if (m_content.size() == m_size) {
      return false;
    }

    m_content[m_size].value = std::move(value);
    ++m_size;

    return true;
  }

  ~static_vector() noexcept(release) {
    namespace stdv = std::views;

    for (auto &elem : m_content | stdv::take(m_size)) {
      std::destroy_at(&elem.value);
    }
  }

private:
  union stored_type {
    value_type value;
  };

  std::array<stored_type, Capacity> m_content;
  std::size_t m_size{};
};

} // namespace upd
