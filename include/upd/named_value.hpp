#pragma once

#include <array>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "description.hpp"
#include "detail/always_false.hpp"
#include "detail/integral_constant.hpp"
#include "detail/tuple_operations.hpp"
#include "detail/variadic/concat.hpp"
#include "integer.hpp"
#include "named_tuple.hpp"
#include "upd.hpp"

namespace upd {

template<typename>
struct kw_t;

template<auto Identifier>
constexpr auto kw = kw_t<detail::integral_constant_t<Identifier>>{};

template<typename Identifier, typename T>
struct named_value {
  static_assert(detail::has_value_member_v<Identifier>, "`Identifier` must have a `value` member");

  constexpr static auto identifier = Identifier::value;

  template<typename F>
  [[nodiscard]] constexpr auto map(F &&f) & {
    decltype(auto) mapped_value = std::invoke(UPD_FWD(f), value);
    return kw<identifier> = UPD_FWD(mapped_value);
  }

  template<typename F>
  [[nodiscard]] constexpr auto map(F &&f) const & {
    decltype(auto) mapped_value = std::invoke(UPD_FWD(f), value);
    return kw<identifier> = UPD_FWD(mapped_value);
  }

  template<typename F>
  [[nodiscard]] constexpr auto map(F &&f) && {
    decltype(auto) mapped_value = std::invoke(UPD_FWD(f), std::move(value));
    return kw<identifier> = UPD_FWD(mapped_value);
  }

  template<typename NamedObject>
  [[nodiscard]] constexpr auto operator,(NamedObject nobj) const & {
    if constexpr (detail::is_instance_of_v<NamedObject, named_value>) {
      auto values = std::tuple{value, std::move(nobj.value)};
      return name_tuple<identifier, nobj.identifier>(std::move(values));
    } else if constexpr (is_named_tuple_instance<NamedObject>::value) {
      auto tail = std::tuple{value};
      auto named_tail = name_tuple<identifier>(std::move(tail));
      return concat_named_tuple(std::move(named_tail), std::move(nobj));
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`nobj` must be a `named_value` or `named_tuple` instance");
    }
  }

  template<typename NamedObject>
  [[nodiscard]] constexpr auto operator,(NamedObject nobj) &&noexcept {
    if constexpr (detail::is_instance_of_v<NamedObject, named_value>) {
      auto values = std::tuple{std::move(value), std::move(nobj.value)};
      return name_tuple<identifier, nobj.identifier>(std::move(values));
    } else if constexpr (is_named_tuple_instance<NamedObject>::value) {
      auto tail = std::tuple{std::move(value)};
      auto named_tail = name_tuple<identifier>(std::move(tail));
      return concat_named_tuple(std::move(named_tail), std::move(nobj));
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`nobj` must be a `named_value` or `named_tuple` instance");
    }
  }

  T value;
};

template<typename Identifier>
struct kw_t {
  static_assert(detail::has_value_member_v<Identifier>, "`Identifier` must have a `value` member");

  constexpr static auto identifier = Identifier::value;

  template<typename T>
  [[nodiscard]] constexpr auto operator=(T x) const noexcept -> named_value<Identifier, T> {
    return named_value<Identifier, T>{UPD_FWD(x)};
  }
};

} // namespace upd