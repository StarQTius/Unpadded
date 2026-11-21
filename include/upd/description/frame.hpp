#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <format>
#include <memory>
#include <memory_resource>
#include <ranges>
#include <tuple>
#include <utility>
#include <vector>

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

template<typename...>
class frame;

template<auto... Identifiers, typename... Ts>
class frame<entry<Identifiers, Ts>...> {
  friend struct record_like_for<frame<entry<Identifiers, Ts>...>>;

  template<typename...>
  friend struct description;

  constexpr static auto buffer_size = 1024;

  using storage_type = record<entry<Identifiers, Ts>...>;

public:
  constexpr frame() = default;

  template<typename... Entries>
  constexpr explicit frame(Entries &&...entries)
      : m_storage{UPD_FWD(entries)...} {}

  constexpr frame(const frame &other) = default;

  constexpr frame(frame &&other) = default;

  constexpr auto operator=(const frame &other) -> frame & = default;

  constexpr auto operator=(frame &&other) -> frame & = default;

  template<typename Self, auto Id>
  [[nodiscard]] constexpr auto
  operator[](this Self &&self, keyword2<Id>) noexcept(release) -> auto && {
    return get<Id>(UPD_FWD(self).m_storage);
  }

private:
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

template<>
struct upd::collector_for<upd::template_box<upd::frame>> {
  template<record_like View>
  [[nodiscard]] constexpr static auto collect(View &&view) {
    namespace updv = upd::record_views;

    return updv::apply(
        [](auto &&...entries) { return frame{UPD_FWD(entries)...}; },
        UPD_FWD(view));
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
