#pragma once

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <utility>

#include "../upd.hpp"
#include "frame_allocator.hpp"

namespace upd {

template<typename T>
class frame_segment {
  template<typename...>
  friend class frame;

public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = const T *;
  using const_pointer = const T *;
  using reference = const T &;
  using const_reference = const T &;
  using iterator = const T *;
  using const_iterator = const T *;
  using reverse_iterator = std::reverse_iterator<const T *>;
  using const_reverse_iterator = std::reverse_iterator<const T *>;

  constexpr frame_segment()
      : m_begin{nullptr}, m_end{nullptr}, m_alloc{nullptr} {}

  constexpr explicit frame_segment(frame_allocator *alloc)
      : m_begin{nullptr}, m_end{nullptr}, m_alloc{alloc} {}

  template<std::ranges::range R>
  constexpr frame_segment(frame_allocator *alloc, R &&r)
      : m_begin{nullptr}, m_end{nullptr}, m_alloc{alloc} {
    for (auto &&elem : UPD_FWD(r)) {
      emplace_back(UPD_FWD(elem));
    }
  }

  constexpr frame_segment(frame_allocator *alloc, std::initializer_list<T> init)
      : m_begin{nullptr}, m_end{nullptr}, m_alloc{alloc} {
    for (auto &&elem : std::move(init)) {
      emplace_back(UPD_FWD(elem));
    }
  }

  constexpr frame_segment(const frame_segment &other)
      : m_begin{nullptr}, m_end{nullptr}, m_alloc{nullptr} {
    *this = other;
  }

  constexpr frame_segment(frame_segment &&other)
      : m_begin{nullptr}, m_end{nullptr}, m_alloc{nullptr} {
    *this = std::move(other);
  }

  constexpr auto operator=(const frame_segment &other) -> frame_segment & {
    if (other.m_alloc && other.m_alloc->m_dest) {
      UPD_ASSERT(!m_alloc);

      m_alloc = other.m_alloc->m_dest;
      for (const auto &v : other) {
        emplace_back(v);
      }
    } else {
      m_begin = other.m_begin;
      m_end = other.m_end;
      m_alloc = other.m_alloc;
    }

    return *this;
  }

  constexpr auto operator=(frame_segment &&other) -> frame_segment & {
    if (other.m_alloc && other.m_alloc->m_dest) {
      UPD_ASSERT(!m_alloc);

      m_alloc = other.m_alloc->m_dest;
      for (const auto &v : other) {
        emplace_back(std::move(v));
      }
    } else {
      m_begin = other.m_begin;
      m_end = other.m_end;
      m_alloc = other.m_alloc;
    }

    return *this;
  }

  constexpr auto operator[](size_type i) const noexcept(release) -> const T & {
    UPD_ASSERT(i < size());
    return m_begin[i];
  }

  constexpr auto front() const noexcept(release) -> const T & {
    UPD_ASSERT(size() > 0);
    return *m_begin;
  }

  constexpr auto back() const noexcept(release) -> const T & {
    UPD_ASSERT(size() > 0);
    return *(m_end - 1);
  }

  [[nodiscard]] constexpr auto data() const noexcept(release) -> const T * {
    return m_begin;
  }

  [[nodiscard]] constexpr auto begin() const noexcept(release) -> const T * {
    return m_begin;
  }

  [[nodiscard]] constexpr auto cbegin() const noexcept(release) -> const T * {
    return m_begin;
  }

  [[nodiscard]] constexpr auto end() const noexcept(release) -> const T * {
    return m_end;
  }

  [[nodiscard]] constexpr auto cend() const noexcept(release) -> const T * {
    return m_end;
  }

  [[nodiscard]] constexpr auto
  rbegin() const noexcept(release) -> const_reverse_iterator {
    return const_reverse_iterator{m_end};
  }

  [[nodiscard]] constexpr auto
  crbegin() const noexcept(release) -> const_reverse_iterator {
    return const_reverse_iterator{m_end};
  }

  [[nodiscard]] constexpr auto
  rend() const noexcept(release) -> const_reverse_iterator {
    return const_reverse_iterator{m_begin};
  }

  [[nodiscard]] constexpr auto
  crend() const noexcept(release) -> const_reverse_iterator {
    return const_reverse_iterator{m_begin};
  }

  [[nodiscard]] constexpr auto size() const noexcept(release) -> std::size_t {
    return m_end - m_begin;
  }

  [[nodiscard]] constexpr auto
  operator==(const frame_segment &other) const -> bool
    requires std::equality_comparable<T>
  {
    return std::ranges::equal(*this, other);
  }

  template<typename... Args>
  constexpr void emplace_back(Args &&...args) noexcept(release) {
    auto *p = m_alloc->construct<T>(UPD_FWD(args)...);
    UPD_ASSERT(!m_end || p == m_end);

    if (!m_begin) {
      m_begin = p;
    }

    m_end = p + 1;
  }

private:
  const T *m_begin;
  const T *m_end;
  frame_allocator *m_alloc;
};

template<typename T>
  requires std::three_way_comparable<T>
[[nodiscard]] constexpr auto
operator<=>(const frame_segment<T> &lhs, const frame_segment<T> &rhs)
    -> std::compare_three_way_result_t<T> {
  return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(),
                                                rhs.begin(), rhs.end());
}

} // namespace upd
