#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "../upd.hpp"
#include "record_like.hpp"

namespace upd {

template<template<typename...> typename View, typename... Args>
struct record_view_adaptor_t {
  constexpr explicit record_view_adaptor_t(std::in_place_t, Args... args) : args{UPD_FWD(args)...} {}

  std::tuple<Args...> args;
};

template<template<typename...> typename View>
constexpr auto record_view_adaptor = [](auto &&...args) {
  return record_view_adaptor_t<View, std::decay_t<decltype(args)>...>{std::in_place, UPD_FWD(args)...};
};

template<record_like Record, template<typename...> typename View, typename... Args>
  requires requires(Record rec, Args... args) { View{UPD_FWD(rec), UPD_FWD(args)...}; }
[[nodiscard]] constexpr auto operator|(Record &&rec, const record_view_adaptor_t<View, Args...> &adaptor) {
  return std::apply([&](auto &&...args) { return View{UPD_FWD(rec), UPD_FWD(args)...}; }, adaptor.args);
}

template<record_like Record, template<typename...> typename View, typename... Args>
  requires requires(Record rec, Args... args) { View{UPD_FWD(rec), UPD_FWD(args)...}; }
[[nodiscard]] constexpr auto operator|(Record &&rec, record_view_adaptor_t<View, Args...> &&adaptor) {
  return std::apply([&](auto &&...args) { return View{UPD_FWD(rec), UPD_FWD(args)...}; }, std::move(adaptor).args);
}

} // namespace upd
