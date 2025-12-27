#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <memory>
#include <ranges>
#include <utility>

#include "upd.hpp"

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

  template<typename U, std::size_t N>
    requires std::convertible_to<U, T>
  constexpr static_vector(const upd::static_vector<U, N> &other) : static_vector{} {
    for (const auto &val : other) {
      push_back(val);
    }
  }

  constexpr static_vector(std::initializer_list<T> init) : static_vector{} {
    for (auto &val : init) {
      push_back(std::move(val));
    }
  }

  constexpr auto operator[](std::size_t i) & noexcept(release) -> auto & {
    return reinterpret_cast<value_type &>(m_content[i]);
  }

  constexpr auto operator[](std::size_t i) && noexcept(release) -> auto && {
    return reinterpret_cast<value_type &&>(m_content[i]);
  }

  constexpr auto operator[](std::size_t i) const & noexcept(release) -> const auto & {
    return reinterpret_cast<const value_type &>(m_content[i]);
  }

  constexpr auto operator[](std::size_t i) const && noexcept(release) -> const auto && {
    return reinterpret_cast<const value_type &&>(m_content[i]);
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

  ~static_vector() {
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

static_assert(std::destructible<upd::static_vector<int, 32>>);

template<typename T, std::size_t Max>
struct std::formatter<upd::static_vector<T, Max>> {
  std::formatter<T> element_formatter;

  consteval formatter() noexcept = default;

  constexpr auto parse(std::format_parse_context &ctx) { return element_formatter.parse(ctx); }

  auto format(const upd::static_vector<T, Max> &statvec, std::format_context &ctx) const {
    auto it = ctx.out();
    it = std::format_to(it, "{{");

    for (auto first = true; const auto &elem : statvec) {
      if (first) {
        first = false;
      } else {
        it = std::format_to(it, ", ");
      }

      ctx.advance_to(it);
      it = element_formatter.format(elem, ctx);
    }

    it = std::format_to(it, "}}");

    return it;
  }
};
