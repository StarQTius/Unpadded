#pragma once

#include "../collector_of.hpp"
#include "../upd.hpp"
#include "tuple_like.hpp"

namespace upd::tuple_views {

template<template<typename...> typename Tuple>
struct to_t {};

template<template<typename...> typename Tuple>
constexpr auto to = to_t<Tuple>{};

template<tuple_like2 View, template<typename...> typename Tuple>
  requires collector_of<Tuple, View>
[[nodiscard]] constexpr auto operator|(View &&view, to_t<Tuple>) {
  return collect<Tuple>(UPD_FWD(view));
}

} // namespace upd::tuple_views
