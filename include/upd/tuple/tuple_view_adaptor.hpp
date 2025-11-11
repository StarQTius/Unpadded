#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "../upd.hpp"
#include "tuple_like.hpp"

namespace upd {

template<template<typename...> typename View, typename... Args>
struct tuple_view_adaptor_t {
  constexpr explicit tuple_view_adaptor_t(std::in_place_t, Args... args) : args{UPD_FWD(args)...} {}

  std::tuple<Args...> args;
};

template<template<typename...> typename View>
constexpr auto tuple_view_adaptor = [](auto &&...args) {
  return tuple_view_adaptor_t<View, std::decay_t<decltype(args)>...>{std::in_place, UPD_FWD(args)...};
};

template<tuple_like2 Tuple, template<typename...> typename View, typename... Args>
  requires requires(Tuple t, Args... args) { View{UPD_FWD(t), UPD_FWD(args)...}; }
[[nodiscard]] constexpr auto operator|(Tuple &&t, const tuple_view_adaptor_t<View, Args...> &adaptor) {
  return std::apply([&](auto &&...args) { return View{UPD_FWD(t), UPD_FWD(args)...}; }, adaptor.args);
}

template<tuple_like2 Tuple, template<typename...> typename View, typename... Args>
  requires requires(Tuple t, Args... args) { View{UPD_FWD(t), UPD_FWD(args)...}; }
[[nodiscard]] constexpr auto operator|(Tuple &&t, tuple_view_adaptor_t<View, Args...> &&adaptor) {
  return std::apply([&](auto &&...args) { return View{UPD_FWD(t), UPD_FWD(args)...}; }, std::move(adaptor).args);
}

} // namespace upd
