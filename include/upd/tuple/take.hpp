#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <ranges>
#include <type_traits>
#include <utility>

#include "../constexpr.hpp"
#include "../get.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<tuple_like2 Base, metavalue Metacount>
struct take_view {
  Base base;

  explicit constexpr take_view(Base b, Metacount) : base{UPD_FWD(b)} {}
};

template<std::size_t Count>
constexpr auto take = tuple_view_adaptor<take_view>(expr<Count>);

template<tuple_like2 Base, typename Pred>
struct take_while_view {
  Base base;
  Pred pred;
};

constexpr auto take_while = tuple_view_adaptor<take_while_view>;

} // namespace upd::tuple_views

template<upd::tuple_like2 Base, upd::metavalue Metacount>
struct upd::tuple_view_for<upd::tuple_views::take_view<Base, Metacount>> {
  using base_type = Base;

  constexpr static auto size = Metacount::value;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return upd::get<I>(UPD_FWD(view).base);
  }
};

template<upd::tuple_like2 Base, typename Pred>
struct upd::tuple_view_for<upd::tuple_views::take_while_view<Base, Pred>> {
  using base_type = Base;

  template<std::size_t I>
  using ith_arg_t = std::remove_reference_t<tuple_element_t<I, Base>>;

  template<std::size_t I>
  using ith_result_t = decltype(std::declval<Pred>().template operator()<ith_arg_t<I>>());

  constexpr static auto size = UPD_WITH_SEQUENCE(Is, tuple_size_v<Base>) {
    namespace stdv = std::views;

    constexpr auto truth_table = std::array{ith_result_t<Is>::value...};
    return std::ranges::count(truth_table | stdv::take_while(std::identity{}), true);
  };

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return upd::get<I>(UPD_FWD(view).base);
  }
};
