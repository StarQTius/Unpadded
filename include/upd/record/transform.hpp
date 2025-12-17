#pragma once

#include <cstddef>

#include "../constexpr.hpp"
#include "../upd.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
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

  constexpr static auto size = record_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) -> decltype(auto) {
    constexpr auto tag = record_tag_v<I, Base>;
    using type = decltype(UPD_INVOKE(view.f, expr<tag>, upd::get_ith<I>(UPD_FWD(view).base)));
    return entry<tag, type>{UPD_INVOKE(view.f, expr<tag>, upd::get_ith<I>(UPD_FWD(view).base))};
  }
};
