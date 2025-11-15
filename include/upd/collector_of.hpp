#pragma once

#include <array>
#include <ranges>
#include <tuple>
#include <utility>
#include <variant>

#include "implementation_of.hpp"
#include "record/record_like.hpp"
#include "record/regular_record.hpp"
#include "tuple/regular_tuple.hpp"
#include "tuple/tuple_like.hpp"
#include "tuple/tuple_size.hpp"
#include "upd.hpp"
#include "with_sequence.hpp"

namespace upd {

template<template<typename...> typename>
struct collector_for; // IWYU pragma: keep

template<template<typename...> typename TT, typename View>
  requires(is_implementation_of<TT, collector_for>() && (tuple_like2<View> || record_like<View>))
[[nodiscard]] constexpr auto collect(View &&view) {
  return collector_for<TT>::collect(UPD_FWD(view));
}

template<template<typename...> typename TT, typename View>
concept collector_of = (tuple_like2<View> && requires(View &&view) {
                         { collect<TT>(UPD_FWD(view)) } -> regular_tuple;
                       }) || (record_like<View> && requires(View &&view) {
                         { collect<TT>(UPD_FWD(view)) } -> regular_record;
                       });

} // namespace upd

template<>
struct upd::collector_for<std::tuple> {
  template<tuple_like2 View>
  [[nodiscard]] constexpr static auto collect(View &&view) {
    return UPD_WITH_SEQUENCE(Is, tuple_size_v<View>, &) { return std::tuple{get<Is>(UPD_FWD(view))...}; };
  }
};
