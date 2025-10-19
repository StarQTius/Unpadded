#pragma once

#include <type_traits>

#include "../named_value.hpp"
#include "../upd.hpp"

namespace upd {

template<auto Identifier, typename T>
struct entry {
  constexpr static auto identifier = Identifier;
  using value_type = T;

  T value;
};

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
