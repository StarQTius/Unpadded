#pragma once

#include <cstddef>
#include <ranges>
#include <utility>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<record_like Base>
struct enumerate_view {
  Base base;
};

template<record_like Base>
enumerate_view(Base &&) -> enumerate_view<Base>;

constexpr auto enumerate = record_view_adaptor<enumerate_view>();

} // namespace upd::record_views

template<upd::record_like Base>
struct upd::record_view_for<upd::record_views::enumerate_view<Base>> {
  using base_type = Base;

  constexpr static auto size = record_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) -> decltype(auto) {
    constexpr auto tag = record_tag_v<I, Base>;
    using type = decltype(upd::get_ith<I>(UPD_FWD(view).base));
    using pair_type = std::pair<auto_constant<I>, type>;
    return entry{expr<tag>, pair_type{expr<I>, upd::get_ith<I>(UPD_FWD(view).base)}};
  }
};
