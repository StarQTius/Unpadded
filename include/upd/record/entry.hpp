#pragma once

#include <concepts>
#include <type_traits>

#include "../constexpr.hpp"
#include "../named_value.hpp"
#include "../upd.hpp"

namespace upd {

template<auto Identifier, typename T>
struct entry {
  constexpr static auto identifier = Identifier;
  using value_type = T;

  template<typename U>
    requires std::constructible_from<T, U>
  explicit constexpr entry(U &&x) : value{UPD_FWD(x)} {}

  template<typename U>
    requires std::constructible_from<T, U>
  explicit constexpr entry(auto_constant<Identifier>, U &&x) : value{UPD_FWD(x)} {}

  T value;
};

template<auto Identifier, typename T>
explicit entry(auto_constant<Identifier>, T) -> entry<Identifier, T>;

template<auto Identifier>
struct keyword2 {
  constexpr static auto identifier = Identifier;

  template<typename T>
  [[nodiscard]] constexpr auto operator=(T &&x) const -> entry<identifier, T> {
    using value_type = std::remove_cvref_t<T>;
    return entry<identifier, value_type>{UPD_FWD(x)};
  }
};

} // namespace upd

namespace upd::literals {

template<name Identifier>
[[nodiscard]] constexpr auto operator""_kw2() noexcept(release) {
  return keyword2<Identifier>{};
}

} // namespace upd::literals
