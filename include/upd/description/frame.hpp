#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <memory>
#include <memory_resource>
#include <tuple>
#include <utility>

#include "../record/as_tuple.hpp"
#include "../record/entry.hpp"
#include "../record/record.hpp"
#include "../record/record_element.hpp"
#include "../record/record_like.hpp"
#include "../record/to.hpp"
#include "../record/transform.hpp"
#include "../tuple/for_each.hpp"
#include "../upd.hpp"
#include "../utility/get.hpp"
#include "../utility/is_instance_of.hpp"
#include "frame_segment.hpp"

namespace upd {

inline struct : std::pmr::memory_resource {
  auto do_allocate(std::size_t, std::size_t) -> void * { return nullptr; }

  void do_deallocate(void *, std::size_t, std::size_t) {}

  bool do_is_equal(const std::pmr::memory_resource &other) const noexcept {
    return &other == this;
  }
} nothrow_null_memory_resource;

template<typename...>
class frame;

template<auto... Identifiers, typename... Ts>
class frame<entry<Identifiers, Ts>...> {
  friend struct upd::record_like_for<
      upd::frame<upd::entry<Identifiers, Ts>...>>;

  constexpr static auto buffer_size = 1024;

  using storage_type = record<entry<Identifiers, Ts>...>;

  template<typename Storage>
  constexpr auto replicate_storage(Storage &&other) {
    using namespace upd::record_views;

    return other
           | transform([&]<auto Id, typename T>(T &&v) -> decltype(auto) {
               using target_type = record_element_t<Id, storage_type>;
               if constexpr (is_instance_of<target_type, frame_segment>()) {
                 using ctor_arg_t =
                     typename target_type::template ctor_argument<T &&>;
                 return ctor_arg_t{&v, &m_rsrc};
               } else {
                 return std::forward_like<Storage>(v);
               }
             })
           | to<record>;
  }

public:
  constexpr frame()
      : m_rsrc{m_buf.data(), buffer_size, &nothrow_null_memory_resource},
        m_storage{storage_type{}} {}

  template<typename... Entries>
  constexpr explicit frame(Entries &&...entries)
      : m_rsrc{m_buf.data(), buffer_size, &nothrow_null_memory_resource},
        m_storage{replicate_storage(record{UPD_FWD(entries)...})} {}

  constexpr frame(const frame &other)
      : m_rsrc{m_buf.data(), buffer_size, &nothrow_null_memory_resource},
        m_storage{replicate_storage(other.m_storage)} {}

  constexpr frame(frame &&other)
      : m_rsrc{m_buf.data(), buffer_size, &nothrow_null_memory_resource},
        m_storage{replicate_storage(std::move(other).m_storage)} {}

  constexpr auto operator=(const frame &other) -> frame & {
    std::destroy_at(&m_storage);
    m_rsrc.release();
    std::construct_at(&m_storage, replicate_storage(other.m_storage));

    return *this;
  }

  constexpr auto operator=(frame &&other) -> frame & {
    std::destroy_at(&m_storage);
    m_rsrc.release();
    std::construct_at(&m_storage,
                      replicate_storage(std::move(other).m_storage));

    return *this;
  }

  template<typename Self, auto Id>
  [[nodiscard]] constexpr auto
  operator[](this Self &&self, keyword2<Id>) noexcept(release) -> auto && {
    return get<Id>(UPD_FWD(self).m_storage);
  }

private:
  std::array<char, buffer_size> m_buf;
  std::pmr::monotonic_buffer_resource m_rsrc;
  storage_type m_storage;
};

template<typename... Entries>
  requires(is_instance_of<Entries, entry>() && ...)
explicit frame(Entries...) -> frame<Entries...>;

} // namespace upd

template<auto... Identifiers, typename... Ts>
struct upd::record_like_for<upd::frame<upd::entry<Identifiers, Ts>...>> {
  using frame_type = upd::frame<upd::entry<Identifiers, Ts>...>;

  constexpr static auto size = sizeof...(Ts);

  template<std::size_t I>
  constexpr static auto tag = std::get<I>(std::tuple{Identifiers...});

  template<auto Id>
  using element_type = record_element_t<Id, typename frame_type::storage_type>;

  template<std::size_t I, typename Record>
  [[nodiscard]] constexpr static auto
  get_ith(Record &&rec) noexcept(release) -> auto && {
    return UPD_FWD(rec)[keyword2<tag<I>>{}];
  }
};

template<typename... Entries>
struct std::formatter<upd::frame<Entries...>> {
  consteval formatter() noexcept(upd::release) = default;

  [[nodiscard]] constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  [[nodiscard]] constexpr static auto
  format(const upd::frame<Entries...> &rec, std::format_context &ctx) {
    namespace updv = upd::record_views;

    auto it = ctx.out();
    it = std::format_to(it, "(");

    auto first = true;
    upd::tuple_views::for_each(rec | updv::as_tuple, [&](const auto &e) {
      if (first) {
        it = std::format_to(it, "{}", e);
        first = false;
      } else {
        it = std::format_to(it, ", {}", e);
      }
    });
    it = std::format_to(it, ")");

    ctx.advance_to(it);
    return it;
  }
};
