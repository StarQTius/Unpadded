#pragma once

#include <cstddef>
#include <ranges>
#include <utility>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<tuple_like2 Base>
struct enumerate_view {
  Base base;
};

template<tuple_like2 Base>
enumerate_view(Base &&) -> enumerate_view<Base>;

constexpr auto enumerate = tuple_view_adaptor<enumerate_view>();

} // namespace upd::tuple_views

template<upd::tuple_like2 Base>
struct upd::tuple_view_for<upd::tuple_views::enumerate_view<Base>> {
  using base_type = Base;

  constexpr static auto size = tuple_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) {
    using type = decltype(upd::get<I>(UPD_FWD(view).base));
    using pair_type = std::pair<auto_constant<I>, type>;
    return pair_type{expr<I>, upd::get<I>(UPD_FWD(view).base)};
  }
};
