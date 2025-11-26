#pragma once

#include "../collector_of.hpp"
#include "../constexpr.hpp"
#include "../upd.hpp"
#include "../variadic/template_box.hpp"
#include "tuple_like.hpp"

namespace upd::tuple_views {

struct to_t {};

template<template<typename...> typename Tuple>
void to(to_t, auto_constant<template_box<Tuple>>) {}

template<template<auto...> typename Tuple>
void to(to_t, auto_constant<template_box<Tuple>>) {}

template<template<typename, auto...> typename Tuple>
void to(to_t, auto_constant<template_box<Tuple>>) {}

template<tuple_like2 View, auto TemplateBox>
  requires collector_of<TemplateBox, View>
[[nodiscard]] constexpr auto operator|(View &&view, void (&)(to_t, auto_constant<TemplateBox>)) {
  return collect<TemplateBox>(UPD_FWD(view));
}

} // namespace upd::tuple_views
