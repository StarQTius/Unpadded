#pragma once

#include <cstddef>
#include <memory>
#include <new>

#include "../upd.hpp"

namespace upd {

class frame_allocator {
  template<typename...>
  friend class frame;

  template<typename>
  friend class frame_segment;

  constexpr frame_allocator(const frame_allocator &) noexcept(release) = delete;
  constexpr frame_allocator(frame_allocator &&) noexcept(release) = delete;

  constexpr auto operator=(const frame_allocator &) noexcept(release)
      -> frame_allocator & = delete;

  constexpr auto
  operator=(frame_allocator &&) noexcept(release) -> frame_allocator & = delete;

  constexpr frame_allocator(char *buf, std::size_t size) noexcept(release)
      : m_pfree{buf}, m_avail{size}, m_dest{nullptr} {};

  constexpr frame_allocator(frame_allocator &alloc,
                            char *buf,
                            std::size_t size) noexcept(release)
      : m_pfree{buf}, m_avail{size}, m_dest{nullptr} {
    alloc.m_dest = this;
  }

  template<typename T, typename... Args>
  [[nodiscard]] constexpr auto
  construct(Args &&...args) noexcept(release) -> T * {
    auto *vp = std::align(sizeof(T), alignof(T), m_pfree, m_avail);
    auto *p = std::launder(reinterpret_cast<T *>(vp));
    if (!p) {
      return nullptr;
    }

    auto *retval = std::construct_at(p, UPD_FWD(args)...);
    m_pfree = p + 1;
    m_avail -= sizeof(T);

    return retval;
  }

  void *m_pfree;
  std::size_t m_avail;
  frame_allocator *m_dest;
};

} // namespace upd
