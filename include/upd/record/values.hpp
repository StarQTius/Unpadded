#pragma once

#include <cstddef>

#include "../tuple/tuple_view.hpp"
#include "../upd.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<record_like Base>
struct values_view {
  Base base;
};

template<record_like Base>
values_view(Base &&) -> values_view<Base>;

constexpr auto values = record_view_adaptor<values_view>();

} // namespace upd::record_views

template<upd::record_like Base>
struct upd::tuple_view_for<upd::record_views::values_view<Base>> {
  using base_type = Base;

  constexpr static auto size = record_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return upd::get_ith<I>(UPD_FWD(view).base);
  }
};
