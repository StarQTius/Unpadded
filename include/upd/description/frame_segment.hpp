#pragma once

#include <algorithm>
#include <compare>
#include <cstddef>
#include <iterator>
#include <memory_resource>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include "../record/record.hpp"
#include "../upd.hpp"

namespace upd {

template<typename T>
class frame_segment {
  template<typename...>
  friend class frame;

  template<typename RangeReference>
  struct ctor_argument {
    std::remove_reference_t<RangeReference> *ptr;
    std::pmr::memory_resource *rsrc;
  };

public:
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = element_type *;
  using const_pointer = const element_type *;
  using reference = element_type &;
  using const_reference = const element_type &;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  template<typename RangeReference>
  constexpr explicit frame_segment(const ctor_argument<RangeReference> &h)
      : m_vec{std::from_range, std::forward<RangeReference>(*h.ptr),
              std::pmr::polymorphic_allocator<T>{h.rsrc}} {}

  constexpr frame_segment(const frame_segment &) = delete;
  constexpr frame_segment(frame_segment &&) = delete;

  constexpr auto operator=(const frame_segment &) -> frame_segment & = delete;
  constexpr auto operator=(frame_segment &&) -> frame_segment & = delete;

  constexpr auto
  operator[](size_type idx) const noexcept(release) -> reference {
    UPD_ASSERT(idx < size());
    return m_vec[idx];
  }

  constexpr auto front() const noexcept(release) -> reference {
    UPD_ASSERT(size() > 0);
    return m_vec.front();
  }

  constexpr auto back() const noexcept(release) -> reference {
    UPD_ASSERT(size() > 0);
    return m_vec.back();
  }

  [[nodiscard]] constexpr auto data() const noexcept(release) -> pointer {
    return m_vec.data();
  }

  // Iterators
  [[nodiscard]] constexpr auto begin() const noexcept(release) -> iterator {
    return m_vec.data();
  }

  [[nodiscard]] constexpr auto
  cbegin() const noexcept(release) -> const_iterator {
    return m_vec.data();
  }

  [[nodiscard]] constexpr auto end() const noexcept(release) -> iterator {
    return m_vec.data() + m_vec.size();
  }

  [[nodiscard]] constexpr auto
  cend() const noexcept(release) -> const_iterator {
    return m_vec.data() + m_vec.size();
  }

  [[nodiscard]] constexpr auto
  rbegin() const noexcept(release) -> reverse_iterator {
    return reverse_iterator{end()};
  }

  [[nodiscard]] constexpr auto
  crbegin() const noexcept(release) -> const_reverse_iterator {
    return const_reverse_iterator{cend()};
  }

  [[nodiscard]] constexpr auto
  rend() const noexcept(release) -> reverse_iterator {
    return reverse_iterator{begin()};
  }

  [[nodiscard]] constexpr auto
  crend() const noexcept(release) -> const_reverse_iterator {
    return const_reverse_iterator{cbegin()};
  }

  [[nodiscard]] constexpr auto size() const noexcept(release) -> size_type {
    return m_vec.size();
  }

  template<typename U>
  [[nodiscard]] constexpr auto
  operator==(const frame_segment<U> &other) const -> bool {
    if (size() != other.size()) {
      return false;
    }

    return std::equal(begin(), end(), other.begin());
  }

  template<typename U>
  [[nodiscard]] constexpr auto operator<=>(const frame_segment<U> &other) const
      -> std::compare_three_way_result_t<T, U>
    requires std::three_way_comparable<T, U>
  {
    return std::lexicographical_compare_three_way(begin(), end(), other.begin(),
                                                  other.end());
  }

private:
  mutable std::pmr::vector<T> m_vec;
};

} // namespace upd
