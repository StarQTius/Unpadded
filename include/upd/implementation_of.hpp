#pragma once

#include <type_traits>

#include "upd.hpp"

namespace upd {

template<typename T, template<typename> typename Traits>
concept implementation_of = requires {
  typename Traits<std::remove_cvref_t<T>>;
  Traits<std::remove_cvref_t<T>>{};
};

template<template<typename...> typename TT, template<template<typename...> typename> typename Traits>
[[nodiscard]] constexpr auto is_implementation_of() noexcept(release) -> bool {
  return requires {
    typename Traits<TT>;
    Traits<TT>{};
  };
}

template<auto X, template<auto> typename Traits>
[[nodiscard]] constexpr auto is_implementation_of() noexcept(release) -> bool {
  return requires {
    typename Traits<X>;
    Traits<X>{};
  };
}

} // namespace upd
