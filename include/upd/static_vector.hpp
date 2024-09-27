#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>

namespace upd {

template<typename T, std::size_t Capacity>
class static_vector {
public:
  using value_type = T;

  constexpr explicit static_vector(std::initializer_list<T> init)
      : static_vector{std::move_iterator{init.begin()}, std::move_iterator{init.end()}} {}

  template<typename It>
  constexpr explicit static_vector(It first, It last) {
    auto range_size = std::distance(first, last);
    UPD_ASSERT(range_size >= 0);
    UPD_ASSERT(static_cast<std::uintmax_t>(range_size) <= m_content.size());

    std::copy(first, last, m_content.begin());
    m_size = range_size;
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
    auto *last_opt = get_opt(m_size);

    if (last_opt) {
      *last_opt = value;
      ++m_size;
    }

    return last_opt;
  }

  [[nodiscard]] constexpr auto try_push_back(T &&value) -> bool {
    auto *last_opt = get_opt(m_size);

    if (last_opt) {
      *last_opt = std::move(value);
      ++m_size;
    }

    return last_opt;
  }

private:
  [[nodiscard]] constexpr auto get_opt(std::size_t i) noexcept -> std::optional<T> * {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    return i < m_content.size() ? &m_content[i] : nullptr;
  }

  [[nodiscard]] constexpr auto get_opt(std::size_t i) const noexcept -> const std::optional<T> * {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    return i < m_content.size() ? &m_content[i] : nullptr;
  }

  std::array<std::optional<T>, Capacity> m_content;
  std::size_t m_size{};
};

} // namespace upd
