#pragma once

#include <utility>

#include "constexpr.hpp"
#include "is_instance_of.hpp"
#include "type_traits.hpp"
#include "upd.hpp"

#define UPD_STATIC_ASSERT(CONDITION, FORMAT, ...) static_assert((CONDITION), FORMAT)

namespace upd {

template<auto = [] {}>
constexpr auto always_false = false;

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

  explicit constexpr lite_record_node(auto_constant<Tag>, const T &v) : value{v} {}

  explicit constexpr lite_record_node(auto_constant<Tag>, T &&v) : value{std::move(v)} {}

  template<typename Self>
  [[nodiscard]] constexpr auto get_by_tag(this Self &&self, auto_constant<Tag>) noexcept(release) -> auto && {
    auto &true_self = static_cast<lite_record_node<Tag, T> &>(self);
    return std::forward_like<Self>(true_self).value;
  }

  [[nodiscard]] constexpr auto find_by_type(typebox<T>) const noexcept(release) -> tag_type { return tag; }

  T value;
};

template<typename...>
struct lite_record;

template<auto... Tags, typename... Ts>
  requires(sizeof...(Tags) == sizeof...(Ts))
struct lite_record<lite_record_node<Tags, Ts>...> : lite_record_node<Tags, Ts>... {
  using lite_record_node<Tags, Ts>::get_by_tag...;
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

template<auto... Tags, typename... Ts>
explicit lite_record(lite_record_node<Tags, Ts>...) -> lite_record<lite_record_node<Tags, Ts>...>;

} // namespace upd
