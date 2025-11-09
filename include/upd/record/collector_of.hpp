#pragma once

#include "../implementation_of.hpp"
#include "../upd.hpp"
#include "record_like.hpp"
#include "regular_record.hpp"

namespace upd {

template<template<typename...> typename Record, typename View>
concept collector_of = record_like<View> && requires(View &&view) {
  { collect<Record>(UPD_FWD(view)) } -> regular_record;
};

template<template<typename...> typename>
struct collector_for; // IWYU pragma: keep

template<template<typename...> typename Record, record_like View>
  requires(is_implementation_of<Record, collector_for>())
[[nodiscard]] constexpr auto collect(View &&view) {
  return collector_for<Record>::collect(UPD_FWD(view));
}

} // namespace upd
