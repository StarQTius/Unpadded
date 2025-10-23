#pragma once

#include "../functional.hpp"
#include "../tuple.hpp"
#include "../upd.hpp"
#include "concepts.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<record_like Base, typename F>
struct transform_view {
  Base base;
  F f;
};

template<record_like Base, typename F>
transform_view(Base &&, F) -> transform_view<Base, F>;

constexpr auto transform = record_view_adaptor<transform_view>;

} // namespace upd::record_views

template<upd::record_like Base, typename F>
struct upd::record_view_for<upd::record_views::transform_view<Base, F>> {
  using base_type = Base;

  template<auto Id, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return UPD_INVOKE(view.f, upd::get<Id>(UPD_FWD(view).base));
  }
};
