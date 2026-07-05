#pragma once

#include "../upd.hpp"
#include "../utility/collector_of.hpp"
#include "../variadic/template_box.hpp"
#include "record_like.hpp"

namespace upd::record_views {

template<template<typename...> typename Record>
struct to_t {};

template<template<typename...> typename Record>
constexpr auto to = to_t<Record>{};

template<record_like View, template<typename...> typename Record>
  requires collector_of<template_box<Record>, View>
[[nodiscard]] constexpr auto operator|(View &&view, to_t<Record>) {
  return collect<Record>(UPD_FWD(view));
}

} // namespace upd::record_views
