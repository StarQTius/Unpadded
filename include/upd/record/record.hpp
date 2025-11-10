#pragma once

#include <cstddef>
#include <tuple>

#include "../constexpr.hpp"
#include "../is_instance_of.hpp"
#include "../upd.hpp"
#include "apply.hpp"
#include "collector_of.hpp"
#include "entry.hpp"
#include "lite_record.hpp"
#include "record_like.hpp"

namespace upd {

template<typename...>
class record;

template<auto... Identifiers, typename... Ts>
class record<entry<Identifiers, Ts>...> {
  friend struct upd::record_like_for<upd::record<upd::entry<Identifiers, Ts>...>>;

  using storage_type = lite_record<lite_record_node<Identifiers, Ts>...>;

public:
  template<typename... Entries>
    requires(sizeof...(Ts) == sizeof...(Entries) && (is_instance_of<Entries, entry>() && ...))
  constexpr explicit record(Entries &&...entries)
      : m_storage{lite_record_node<Identifiers, typename Entries::value_type>{UPD_FWD(UPD_FWD(entries).value)}...} {}

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

} // namespace upd

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
struct upd::collector_for<upd::record> {
  template<record_like View>
  [[nodiscard]] constexpr static auto collect(View &&view) {
    namespace updv = upd::record_views;

    return updv::apply([](auto &&...entries) { return record{UPD_FWD(entries)...}; }, UPD_FWD(view));
  }
};
