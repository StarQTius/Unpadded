#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <format>
#include <type_traits>
#include <utility>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/equivalent_to.hpp"
#include "name.hpp"
#include "record_like.hpp"

namespace upd {

template<auto Identifier, typename T>
struct entry {
  constexpr static auto identifier = Identifier;
  using value_type = T;

  constexpr entry()
    requires std::default_initializable<T>
  = default;

  template<typename U>
    requires std::constructible_from<T, U>
  explicit constexpr entry(U &&x) : value{UPD_FWD(x)} {}

  template<typename U>
    requires std::constructible_from<T, U>
  explicit constexpr entry(auto_constant<Identifier>, U &&x)
      : value{UPD_FWD(x)} {}

  [[nodiscard]] constexpr auto forward() & noexcept(release) -> T & {
    return static_cast<T &>(value);
  }

  [[nodiscard]] constexpr auto forward() && noexcept(release) -> T && {
    return static_cast<T &&>(value);
  }

  [[nodiscard]] constexpr auto
  forward() const & noexcept(release) -> const T & {
    return static_cast<const T &>(value);
  }

  [[nodiscard]] constexpr auto
  forward() const && noexcept(release) -> const T && {
    return static_cast<const T &&>(value);
  }

  T value;
};

template<auto Identifier, typename T>
explicit entry(auto_constant<Identifier>, T) -> entry<Identifier, T>;

template<auto Identifier>
struct keyword2 {
  constexpr static auto identifier = Identifier;

  template<typename T>
  [[nodiscard]] constexpr auto operator=(T &&x) const {
    using value_type = std::decay_t<T>;
    return entry<identifier, value_type>{UPD_FWD(x)};
  }

  template<typename T, std::size_t N>
  [[nodiscard]] constexpr auto operator=(T (&a)[N]) const {
    using value_type = std::array<T, N>;
    auto value = std::to_array(a);
    auto retval = entry<identifier, value_type>{value};
  }

  template<typename T, std::size_t N>
  [[nodiscard]] constexpr auto operator=(T (&&a)[N]) const {
    using value_type = std::array<T, N>;
    auto value = std::to_array(std::move(a));
    return entry<identifier, value_type>{std::move(value)};
  }
};

} // namespace upd

namespace upd::literals {

template<name Identifier>
[[nodiscard]] constexpr auto operator""_kw2() noexcept(release) {
  return keyword2<Identifier>{};
}

} // namespace upd::literals

template<auto Identifier, typename T>
struct std::formatter<upd::entry<Identifier, T>> {
  consteval formatter() noexcept(upd::release) = default;

  [[nodiscard]] constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  [[nodiscard]] constexpr static auto
  format(const upd::entry<Identifier, T> &e, std::format_context &ctx) {
    return std::format_to(ctx.out(), "{} -> {}", Identifier, e.value);
  }
};

template<auto Identifier, typename T>
struct upd::record_like_for<upd::entry<Identifier, T>> {
  constexpr static auto size = 1uz;

  template<std::size_t I>
    requires(I == 0)
  constexpr static auto tag = Identifier;

  template<auto Tag>
    requires equivalent_to<Tag, Identifier>
  using element_type = T;

  template<std::size_t I, typename Entry>
    requires(I == 0)
  [[nodiscard]] constexpr static auto
  get_ith(Entry &&ent) noexcept(release) -> auto && {
    return UPD_FWD(ent).forward();
  }
};
