#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <variant>

#include "../record/entry.hpp"
#include "../record/record_view.hpp"
#include "../record/record_view_adaptor.hpp"
#include "../tuple/filter.hpp"
#include "../tuple/to.hpp"
#include "../tuple/transform.hpp"
#include "../tuple/typelist.hpp"
#include "../upd.hpp"
#include "../utility/collector_of.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "../utility/with_sequence.hpp"
#include "../variadic/template_box.hpp"
#include "enumerate.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"
#include "unique.hpp"

namespace upd {

template<typename Base, std::size_t... Indices>
struct group {
  Base base;
  std::tuple<expr_t<Indices>...> indices;
};

template<typename Base, std::size_t... Indices>
group(Base &&, std::tuple<expr_t<Indices>...>) -> group<Base, Indices...>;

} // namespace upd

namespace upd::tuple_views {

template<tuple_like2 Base, typename Identificator>
struct group_by_view {
  Base base;
  Identificator identificator;
};

template<tuple_like2 Base, typename Identificator>
group_by_view(Base &&, Identificator) -> group_by_view<Base, Identificator>;

constexpr auto group_by = tuple_view_adaptor<group_by_view>;

} // namespace upd::tuple_views

template<upd::tuple_like2 Base, typename Identificator>
struct upd::record_view_for<
    upd::tuple_views::group_by_view<Base, Identificator>> {
  using base_type = Base;

  constexpr static auto element_types =
      collect_result_t<template_box<typelist2_t>, Base>{};

  constexpr static auto tags = UPD_WITH_SEQUENCE(Is, tuple_size_v<Base>) {
    namespace updv = upd::tuple_views;

    return element_types
           | updv::transform_type(Identificator{})
           | updv::unique
           | updv::to<std::tuple>;
  };

  constexpr static auto size = tuple_size_v<decltype(tags)>;

  constexpr static auto indices = UPD_WITH_SEQUENCE(Is, size) {
    namespace updv = upd::tuple_views;

    auto indices_and_tags =
        element_types | updv::transform_type(Identificator{}) | updv::enumerate;

    return std::make_tuple(
        (indices_and_tags
         | updv::filter([]<typename IAndExpr> {
             return std::same_as<
                 std::remove_cvref_t<typename IAndExpr::second_type>,
                 tuple_element_t<Is, decltype(tags)>>;
           })
         | updv::transform_type([]<typename IAndExpr> {
             using expression_t = typename IAndExpr::first_type;
             return expression_t{};
           })
         | updv::to<std::tuple>)...);
  };

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) -> decltype(auto) {
    return entry{get<I>(tags), group{UPD_FWD(view).base, get<I>(indices)}};
  }
};

template<typename Base, std::size_t... Indices>
struct upd::tuple_view_for<upd::group<Base, Indices...>> {
  using base_type = Base;

  constexpr static auto indices = std::array{Indices...};

  constexpr static auto size = sizeof...(Indices);

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return upd::get<indices[I]>(UPD_FWD(view).base);
  }
};
