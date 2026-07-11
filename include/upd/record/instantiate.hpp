#pragma once

#include "../utility/collector_of.hpp"
#include "../utility/constexpr.hpp"
#include "../variadic/template_box.hpp"
#include "record_like.hpp"

namespace upd::record_views {

struct instantiate_t {};

template<template<typename...> typename Record>
void instantiate(instantiate_t, auto_constant<template_box<Record>>) {}

template<template<auto...> typename Record>
void instantiate(instantiate_t, auto_constant<template_box<Record>>) {}

template<template<typename, auto...> typename Record>
void instantiate(instantiate_t, auto_constant<template_box<Record>>) {}

template<record_like View, auto TemplateBox>
  requires collector_of<TemplateBox, View>
[[nodiscard]] constexpr auto
operator|(View &&view, void (&)(instantiate_t, auto_constant<TemplateBox>))
    -> collect_result_t<TemplateBox, View>;

} // namespace upd::record_views
