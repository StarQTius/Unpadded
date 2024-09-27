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

template<name>
struct kw_t;

template<name Identifier>
constexpr auto kw = kw_t<Identifier>{};

template<name Identifier, typename T>
class named_value {
public:
  constexpr static auto identifier = Identifier;

  template<typename... Args>
  constexpr explicit named_value(std::in_place_t, Args &&... args): m_value{UPD_FWD(args)...} {}

  template<typename F>
  [[nodiscard]] constexpr auto map(F &&f) & {
    decltype(auto) mapped_value = std::invoke(UPD_FWD(f), m_value);
    return kw<identifier> = UPD_FWD(mapped_value);
  }

  template<typename F>
  [[nodiscard]] constexpr auto map(F &&f) const & {
    decltype(auto) mapped_value = std::invoke(UPD_FWD(f), m_value);
    return kw<identifier> = UPD_FWD(mapped_value);
  }

  template<typename F>
  [[nodiscard]] constexpr auto map(F &&f) && {
    decltype(auto) mapped_value = std::invoke(UPD_FWD(f), std::move(m_value));
    return kw<identifier> = UPD_FWD(mapped_value);
  }

  [[nodiscard]] constexpr auto value() const & noexcept -> const T& {
    return m_value;
  }

  [[nodiscard]] constexpr auto value() && noexcept -> T {
    return std::move(m_value);
  }

  template<typename NamedObject>
  [[nodiscard]] constexpr auto operator,(NamedObject nobj) const & {
    if constexpr (requires { upd::named_value{nobj}; }) {
      auto values = std::tuple{m_value, std::move(nobj).value()};
      return name_tuple<identifier, nobj.identifier>(std::move(values));
    } else if constexpr (is_named_tuple_instance<NamedObject>::value) {
      auto tail = std::tuple{m_value};
      auto named_tail = name_tuple<identifier>(std::move(tail));
      return concat_named_tuple(std::move(named_tail), std::move(nobj));
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`nobj` must be a `named_value` or `named_tuple` instance");
    }
  }

  template<typename NamedObject>
  [[nodiscard]] constexpr auto operator,(NamedObject nobj) &&noexcept {
    if constexpr (requires { upd::named_value{nobj}; }) {
      auto values = std::tuple{std::move(m_value), std::move(nobj).value()};
      return name_tuple<identifier, nobj.identifier>(std::move(values));
    } else if constexpr (is_named_tuple_instance<NamedObject>::value) {
      auto tail = std::tuple{std::move(m_value)};
      auto named_tail = name_tuple<identifier>(std::move(tail));
      return concat_named_tuple(std::move(named_tail), std::move(nobj));
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`nobj` must be a `named_value` or `named_tuple` instance");
    }
  }

private:
  T m_value;
};

template<name Identifier>
struct kw_t {
  constexpr static auto identifier = Identifier;

  template<typename T>
  [[nodiscard]] constexpr auto operator=(T x) const noexcept -> named_value<Identifier, T> {
    return named_value<Identifier, T>{std::in_place, UPD_FWD(x)};
  }
};

} // namespace upd
