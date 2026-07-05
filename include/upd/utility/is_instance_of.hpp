#pragma once

#include "../upd.hpp"

namespace upd {

template<typename T, template<typename...> typename TT>
[[nodiscard]] constexpr auto is_instance_of() noexcept(release) -> bool {
  auto checker = []<typename... Ts>(const TT<Ts...> &) {};
  return requires(const T &x) { checker(x); };
}

template<typename T, template<auto, typename...> typename TT>
[[nodiscard]] constexpr auto is_instance_of() noexcept(release) -> bool {
  return requires(const T &x) { []<auto V>(const TT<V> &) {}(x); }
         || requires(const T &x) { []<auto V, typename... Ts>(const TT<V, Ts...> &) {}(x); };
}

template<typename T, template<template<typename...> typename, typename...> typename TT>
[[nodiscard]] constexpr auto is_instance_of() noexcept(release) -> bool {
  auto checker = []<template<typename...> typename TU, typename... Ts>(const TT<TU, Ts...> &) {};
  return requires(const T &x) { checker(x); };
}

} // namespace upd
