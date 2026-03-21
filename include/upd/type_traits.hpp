#pragma once

#include <concepts>
#include <type_traits>

#include "upd.hpp"

namespace upd {

template<typename T>
concept metavalue = std::is_empty_v<T> && requires { T::value; };

template<typename T>
struct typebox {
  using type = T;
  using unqualified_type = std::remove_cvref_t<T>;

  template<typename U>
  [[nodiscard]] constexpr auto operator==(typebox<U>) noexcept(release) -> bool {
    return std::same_as<T, U>;
  }
};

} // namespace upd
