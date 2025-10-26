#pragma once

#include <cstddef>
#include <type_traits>

#include "../constexpr.hpp"
#include "../functional.hpp"
#include "../upd.hpp"
#include "concepts.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "record_view_adaptor.hpp"
#include "record_view_for.hpp"

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

  constexpr static auto size = record_size_v<std::remove_cvref_t<Base>>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) -> decltype(auto) {
    constexpr auto tag = record_tag_v<I, std::remove_cvref_t<Base>>;
    return entry{expr<tag>, UPD_INVOKE(view.f, upd::get_ith<I>(UPD_FWD(view).base))};
  }
};
