#pragma once

#include <cstddef>

#include "../functional.hpp"
#include "../upd.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<tuple_like2 Base, typename F>
struct transform_view {
  Base base;
  F f;
};

template<tuple_like2 Base, typename F>
transform_view(Base &&, F) -> transform_view<Base, F>;

constexpr auto transform = tuple_view_adaptor<transform_view>;

} // namespace upd::tuple_views

template<upd::tuple_like2 Base, typename F>
struct upd::tuple_view_for<upd::tuple_views::transform_view<Base, F>> {
  using base_type = Base;

  constexpr static auto size = upd::tuple_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return UPD_INVOKE(view.f, upd::get<I>(UPD_FWD(view).base));
  }
};
