#pragma once

#include "../collector_of.hpp"
#include "../constexpr.hpp"
#include "../variadic/template_box.hpp"
#include "tuple_like.hpp"

namespace upd::tuple_views {

struct instantiate_t {};

template<template<typename...> typename Tuple>
void instantiate(instantiate_t, auto_constant<template_box<Tuple>>) {}

template<template<auto...> typename Tuple>
void instantiate(instantiate_t, auto_constant<template_box<Tuple>>) {}

template<template<typename, auto...> typename Tuple>
void instantiate(instantiate_t, auto_constant<template_box<Tuple>>) {}

template<tuple_like2 View, auto TemplateBox>
  requires collector_of<TemplateBox, View>
[[nodiscard]] constexpr auto operator|(View &&view, void (&)(instantiate_t, auto_constant<TemplateBox>))
    -> collect_result_t<TemplateBox, View>;

} // namespace upd::tuple_views
