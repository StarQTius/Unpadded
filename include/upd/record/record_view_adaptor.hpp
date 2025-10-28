#pragma once

#include <tuple>
#include <utility>

#include "../upd.hpp"
#include "concepts.hpp"

namespace upd {

template<template<typename...> typename View, typename... Args>
struct record_view_adaptor_t {
  constexpr explicit record_view_adaptor_t(Args... args) : args{UPD_FWD(args)...} {}

  record_view_adaptor_t(const record_view_adaptor_t &) = delete;
  record_view_adaptor_t(record_view_adaptor_t &&) = delete;

  record_view_adaptor_t &operator=(const record_view_adaptor_t &) = delete;
  record_view_adaptor_t &operator=(record_view_adaptor_t &&) = delete;

  std::tuple<Args...> args;
};

template<template<typename...> typename View>
constexpr auto record_view_adaptor =
    [](auto &&...args) { return record_view_adaptor_t<View, decltype(args) &&...>{UPD_FWD(args)...}; };

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
