#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <utility>

#include "../always_false.hpp"
#include "../constexpr.hpp"
#include "../is_instance_of.hpp"
#include "../static_assert.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "record_element.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<auto Tag>
struct lite_record_tag_node {
  using tag_type = decltype(Tag);

  constexpr static auto tag = Tag;

  [[nodiscard]] constexpr static auto has_tag(auto_constant<Tag>) noexcept(release) -> bool { return true; }
};

template<typename T>
struct lite_record_type_node {
  using value_type = T;

  [[nodiscard]] constexpr static auto has_type(typebox<T>) noexcept(release) -> bool { return true; }
};

template<auto Tag, typename T>
struct lite_record_node : lite_record_tag_node<Tag>, lite_record_type_node<T> {
  using tag_type = decltype(Tag);
  using value_type = T;

  constexpr static auto tag = Tag;

  template<typename U>
    requires std::convertible_to<U, T>
  explicit constexpr lite_record_node(U &&v) : value{UPD_FWD(v)} {}

  template<typename U>
    requires std::convertible_to<U, T>
  explicit constexpr lite_record_node(auto_constant<Tag>, U &&v) : value{UPD_FWD(v)} {}

  template<typename Other>
    requires(is_instance_of<Other, lite_record_node>() && std::convertible_to<typename Other::value_type, T>)
  explicit constexpr lite_record_node(Other &&other) : value(UPD_FWD(other).value) {}

  [[nodiscard]] constexpr auto get_by_tag(auto_constant<Tag>) & noexcept(release) -> T & { return value; }

  [[nodiscard]] constexpr auto get_by_tag(auto_constant<Tag>) const & noexcept(release) -> const T & { return value; }

  [[nodiscard]] constexpr auto get_by_tag(auto_constant<Tag>) && noexcept(release) -> T && {
    return std::forward<T>(value);
  }

  [[nodiscard]] constexpr auto get_by_tag(auto_constant<Tag>) const && noexcept(release) -> const T && {
    return std::forward<const T>(value);
  }

  [[nodiscard]] constexpr static auto get_type_by_tag(auto_constant<Tag>) noexcept(release) -> typebox<T> {
    return typebox<T>{};
  }

  [[nodiscard]] constexpr auto find_by_type(typebox<T>) const noexcept(release) -> tag_type { return tag; }

  T value;
};

template<auto Tag, typename T>
lite_record_node(auto_constant<Tag>, T) -> lite_record_node<Tag, T>;

template<typename...>
struct lite_record;

template<auto... Tags, typename... Ts>
  requires(sizeof...(Tags) == sizeof...(Ts))
struct lite_record<lite_record_node<Tags, Ts>...> : lite_record_node<Tags, Ts>... {
  using lite_record_node<Tags, Ts>::get_by_tag...;
  using lite_record_node<Tags, Ts>::get_type_by_tag...;
  using lite_record_node<Tags, Ts>::find_by_type...;
  using lite_record_node<Tags, Ts>::has_tag...;
  using lite_record_node<Tags, Ts>::has_type...;

  template<typename... Nodes>
    requires(is_instance_of<Nodes, lite_record_node>() && ...)
  explicit constexpr lite_record(Nodes &&...nodes) : lite_record_node<Tags, Ts>{UPD_FWD(nodes)}... {}

  template<auto Tag>
  constexpr static void get_by_tag(auto_constant<Tag>) noexcept(release) {
    UPD_STATIC_ASSERT(always_false<>, "Lite record does not contain '{}' tag", Tag);
  }

  template<auto Tag>
  constexpr static void get_type_by_tag(auto_constant<Tag>) noexcept(release) {
    UPD_STATIC_ASSERT(always_false<>, "Lite record does not contain '{}' tag", Tag);
  }

  template<typename T>
  constexpr static void find_by_type(typebox<T>) noexcept(release) {
    UPD_STATIC_ASSERT(always_false<>, "Lite record does not contain any '{}' value", typeid(T));
  }

  template<auto Tag>
  [[nodiscard]] constexpr static auto has_tag(auto_constant<Tag>) noexcept(release) -> bool {
    return false;
  }

  template<typename T>
  [[nodiscard]] constexpr static auto has_type(typebox<T>) noexcept(release) -> bool {
    return false;
  }
};

template<typename... Nodes>
  requires(is_instance_of<Nodes, lite_record_node>() && ...)
explicit lite_record(Nodes...) -> lite_record<Nodes...>;

template<auto Tag, typename... Nodes>
struct record_element<Tag, lite_record<Nodes...>> {
  using type = typename decltype(lite_record<Nodes...>::get_type_by_tag(expr<Tag>))::type;
};

template<std::size_t I, typename... Nodes>
struct record_tag<I, lite_record<Nodes...>> {
  constexpr static auto value = std::get<I>(std::tuple{Nodes::tag...});
};

template<typename... Nodes>
struct record_size<lite_record<Nodes...>> {
  constexpr static auto value = sizeof...(Nodes);
};

template<auto Tag, typename Record>
  requires(is_instance_of<Record, lite_record>())
[[nodiscard]] constexpr auto get(Record &&rec) noexcept(release) -> auto && {
  return UPD_FWD(rec).get_by_tag(expr<Tag>);
}

template<typename Record>
  requires(is_instance_of<Record, lite_record>())
[[nodiscard]] constexpr static auto collect(Record &&rec) {
  return UPD_WITH_SEQUENCE(Is, record_size_v<Record>, &) {
    return lite_record{lite_record_node{expr<record_tag_v<Is, Record>>, get_ith<Is>(UPD_FWD(rec))}...};
  };
}

template<auto Tag, typename... Nodes>
[[nodiscard]] constexpr static auto has_tag(const lite_record<Nodes...> &rec) noexcept(release) -> bool {
  return rec.has_tag(expr<Tag>);
}

template<auto Tag, typename... Nodes>
[[nodiscard]] constexpr static auto has_type(const lite_record<Nodes...> &rec) noexcept(release) -> bool {
  return rec.has_type(expr<Tag>);
}

} // namespace upd
