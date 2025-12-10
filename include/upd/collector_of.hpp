#pragma once

#include <array>
#include <tuple>
#include <type_traits>
#include <utility>

#include "get.hpp"
#include "implementation_of.hpp"
#include "record/record_like.hpp"
#include "record/regular_record.hpp"
#include "tuple/regular_tuple.hpp"
#include "tuple/tuple_element.hpp"
#include "tuple/tuple_like.hpp"
#include "tuple/tuple_size.hpp"
#include "upd.hpp"
#include "variadic/template_box.hpp"
#include "with_sequence.hpp"

namespace upd {

template<auto>
struct collector_for; // IWYU pragma: keep

template<auto TemplateBox, typename View>
  requires(is_implementation_of<TemplateBox, collector_for>() && (tuple_like2<View> || record_like<View>))
[[nodiscard]] constexpr auto collect(View &&view) {
  return collector_for<TemplateBox>::collect(UPD_FWD(view));
}

template<template<typename...> typename TT, typename View>
  requires(is_implementation_of<template_box<TT>, collector_for>() && (tuple_like2<View> || record_like<View>))
[[nodiscard]] constexpr auto collect(View &&view) {
  return collector_for<template_box<TT>>::collect(UPD_FWD(view));
}

template<template<typename, auto...> typename TT, typename View>
  requires(is_implementation_of<template_box<TT>, collector_for>() && (tuple_like2<View> || record_like<View>))
[[nodiscard]] constexpr auto collect(View &&view) {
  return collector_for<template_box<TT>>::collect(UPD_FWD(view));
}

template<auto TemplateBox, typename View>
concept collector_of = (tuple_like2<View> && requires(View &&view) {
                         { collect<TemplateBox>(UPD_FWD(view)) } -> regular_tuple;
                         { collect<TemplateBox>(std::declval<View>()) } -> regular_tuple;
                       }) || (record_like<View> && requires(View &&view) {
                         { collect<TemplateBox>(UPD_FWD(view)) } -> regular_record;
                         { collect<TemplateBox>(std::declval<View>()) } -> regular_record;
                       });

template<auto TemplateBox, typename View>
  requires tuple_like2<View> || record_like<View>
struct collect_result {
  using type = decltype(collect<TemplateBox>(std::declval<View>()));
};

template<auto TemplateBox, typename View>
  requires tuple_like2<View> || record_like<View>
using collect_result_t = typename collect_result<TemplateBox, View>::type;

} // namespace upd

template<>
struct upd::collector_for<upd::template_box<std::tuple>> {
  template<tuple_like2 View>
  [[nodiscard]] constexpr static auto collect(View &&view) {
    return UPD_WITH_SEQUENCE(Is, tuple_size_v<View>, &) { return std::tuple{get<Is>(UPD_FWD(view))...}; };
  }
};

template<>
struct upd::collector_for<upd::template_box<std::array>> {
  template<tuple_like2 View>
  [[nodiscard]] constexpr static auto collect(View &&view) {
    return UPD_WITH_SEQUENCE(Is, tuple_size_v<View>, &) {
      using common_type = std::common_reference_t<tuple_element_t<Is, View>...>;
      return std::array<common_type, sizeof...(Is)>{get<Is>(UPD_FWD(view))...};
    };
  }
};
