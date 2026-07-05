#pragma once

#include <concepts>
#include <cstddef>
#include <format>
#include <tuple>
#include <type_traits>

#include "../tuple/for_each.hpp"
#include "../upd.hpp"
#include "../utility/collector_of.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "../utility/is_instance_of.hpp"
#include "../utility/with_sequence.hpp"
#include "../variadic/identical.hpp"
#include "../variadic/template_box.hpp"
#include "apply.hpp"
#include "as_tuple.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "lite_record.hpp"
#include "record_element.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "tags_of.hpp"

namespace upd {

template<typename...>
class record;

template<auto... Identifiers, typename... Ts>
class record<entry<Identifiers, Ts>...> {
  friend struct upd::record_like_for<upd::record<upd::entry<Identifiers, Ts>...>>;

  using storage_type = lite_record<lite_record_node<Identifiers, Ts>...>;

public:
  constexpr record()
    requires(std::default_initializable<Ts> && ...)
  = default;

  template<typename... Entries>
    requires(sizeof...(Ts) == sizeof...(Entries) && (is_instance_of<Entries, entry>() && ...))
  constexpr explicit record(Entries &&...entries)
      : m_storage{lite_record_node<Identifiers, typename std::remove_cvref_t<Entries>::value_type>{
            UPD_FWD(entries).forward()}...} {}

  template<typename Record>
    requires(is_instance_of<Record, record>()
             && variadic::identical(std::tuple{expr<Identifiers>...}, tags_of_v<Record>))
  constexpr record(Record &&other)
    requires(std::constructible_from<Ts, decltype(get<Identifiers>(UPD_FWD(other)))> && ...)
      : m_storage{lite_record_node<Identifiers, record_element_t<Identifiers, Record>>{
            get<Identifiers>(UPD_FWD(other))}...} {}

  template<typename Record>
    requires(is_instance_of<Record, record>()
             && variadic::identical(std::tuple{expr<Identifiers>...}, tags_of_v<Record>))
  constexpr record &operator=(Record && other)
    requires(std::assignable_from<Ts, decltype(get<Identifiers>(UPD_FWD(other)))> && ...)
  {
    ((void)(get<Identifiers>(*this) = get<Identifiers>(UPD_FWD(other))), ...);
    return *this;
  }

  template<typename Self, auto Id>
  [[nodiscard]] constexpr auto operator[](this Self &&self, keyword2<Id>) noexcept(release) -> auto && {
    return UPD_FWD(self).m_storage.get_by_tag(expr<Id>);
  }

private:
  storage_type m_storage;
};

template<typename... Entries>
  requires(is_instance_of<Entries, entry>() && ...)
explicit record(Entries...) -> record<Entries...>;

template<typename... Es, typename... Fs>
[[nodiscard]] constexpr auto operator==(const record<Es...> &lhs, const record<Fs...> &rhs) noexcept(release) -> bool {
  auto lhs_tags = std::tuple{expr<Es::identifier>...};
  auto rhs_tags = std::tuple{expr<Fs::identifier>...};
  if constexpr (variadic::identical(lhs_tags, rhs_tags)) {
    return ((get<Es::identifier>(lhs) == get<Es::identifier>(rhs)) && ...);
  } else {
    return false;
  }
}

} // namespace upd

namespace upd::record_operators {

template<typename Lhs, typename Rhs>
  requires(is_instance_of<Lhs, entry>() && is_instance_of<Rhs, entry>())
[[nodiscard]] constexpr auto operator,(Lhs &&lhs, Rhs &&rhs) {
  return record{UPD_FWD(lhs), UPD_FWD(rhs)};
}

template<typename Record, typename Entry>
  requires(is_instance_of<Record, record>() && is_instance_of<Entry, entry>())
[[nodiscard]] constexpr auto operator,(Record &&rec, Entry &&ent) {
  return UPD_WITH_SEQUENCE(Is, record_size_v<Record>, &) {
    return record{entry{expr<record_tag_v<Is, Record>>, get_ith<Is>(UPD_FWD(rec))}..., UPD_FWD(ent)};
  };
}

} // namespace upd::record_operators

template<auto... Identifiers, typename... Ts>
struct upd::record_like_for<upd::record<upd::entry<Identifiers, Ts>...>> {
  using record_type = upd::record<upd::entry<Identifiers, Ts>...>;

  constexpr static auto size = sizeof...(Ts);

  template<std::size_t I>
  constexpr static auto tag = std::get<I>(std::tuple{Identifiers...});

  template<auto Id>
  using element_type = typename decltype(record_type::storage_type::get_type_by_tag(expr<Id>))::type;

  template<std::size_t I, typename Record>
  [[nodiscard]] constexpr static auto get_ith(Record &&rec) noexcept(release) -> auto && {
    return UPD_FWD(rec)[keyword2<tag<I>>{}];
  }
};

template<>
struct upd::collector_for<upd::template_box<upd::record>> {
  template<record_like View>
  [[nodiscard]] constexpr static auto collect(View &&view) {
    namespace updv = upd::record_views;

    return updv::apply([](auto &&...entries) { return record{UPD_FWD(entries)...}; }, UPD_FWD(view));
  }
};

template<typename... Entries>
struct std::formatter<upd::record<Entries...>> {
  consteval formatter() noexcept(upd::release) = default;

  [[nodiscard]] constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  [[nodiscard]] constexpr static auto format(const upd::record<Entries...> &rec, std::format_context &ctx) {
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
