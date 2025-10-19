#pragma once

#include <cstddef>

#include "../constexpr.hpp"
#include "../is_instance_of.hpp"
#include "../upd.hpp"
#include "concepts.hpp"
#include "entry.hpp"
#include "lite_record.hpp"

namespace upd {

template<typename...>
class record;

template<auto... Identifiers, typename... Ts>
class record<entry<Identifiers, Ts>...> {
  template<auto, typename>
  friend struct record_element;

  template<std::size_t, typename>
  friend struct record_tag;

  template<typename>
  friend struct record_size;

  template<auto, typename... Entries>
  friend constexpr auto get(record<Entries...> &rec) -> auto &;

  template<auto, typename... Entries>
  friend constexpr auto get(const record<Entries...> &rec) -> const auto &;

  template<auto, typename... Entries>
  friend constexpr auto get(record<Entries...> &&rec) -> auto &&;

  template<auto, typename... Entries>
  friend constexpr auto get(const record<Entries...> &&rec) -> const auto &&;

  using storage_type = lite_record<lite_record_node<Identifiers, Ts>...>;

public:
  template<typename... Entries>
    requires(sizeof...(Ts) == sizeof...(Entries) && (is_instance_of<Entries, entry>() && ...))
  constexpr explicit record(Entries &&...entries)
      : m_storage{lite_record_node{expr<Identifiers>, UPD_FWD(entries).value}...} {}

  template<typename Self, auto Id>
  [[nodiscard]] constexpr auto operator[](this Self &&self, keyword2<Id>) noexcept(release) -> auto && {
    return get<Id>(UPD_FWD(self));
  }

private:
  storage_type m_storage;
};

template<typename... Entries>
  requires(is_instance_of<Entries, entry>() && ...)
explicit record(Entries...) -> record<Entries...>;

template<auto Id, typename... Entries>
struct record_element<Id, record<Entries...>> {
  using type = record_element_t<Id, typename record<Entries...>::storage_type>;
};

template<std::size_t I, typename... Entries>
struct record_tag<I, record<Entries...>> {
  constexpr static auto value = record_tag_v<I, typename record<Entries...>::storage_type>;
  ;
};

template<typename... Entries>
struct record_size<record<Entries...>> {
  constexpr static auto value = sizeof...(Entries);
};

template<auto Id, typename... Entries>
[[nodiscard]] constexpr auto get(record<Entries...> &rec) noexcept(release) -> auto & {
  return get<Id>(UPD_FWD(rec).m_storage);
}

template<auto Id, typename... Entries>
[[nodiscard]] constexpr auto get(const record<Entries...> &rec) noexcept(release) -> const auto & {
  return get<Id>(UPD_FWD(rec).m_storage);
}

template<auto Id, typename... Entries>
[[nodiscard]] constexpr auto get(record<Entries...> &&rec) noexcept(release) -> auto && {
  return get<Id>(UPD_FWD(rec).m_storage);
}

template<auto Id, typename... Entries>
[[nodiscard]] constexpr auto get(const record<Entries...> &&rec) noexcept(release) -> const auto && {
  return get<Id>(UPD_FWD(rec).m_storage);
}

} // namespace upd
