#pragma once

#include <cstddef>

#include "../upd.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<tuple_like2 Base>
struct reverse_view {
  Base base;
};

template<tuple_like2 Base>
reverse_view(Base &&) -> reverse_view<Base>;

constexpr auto reverse = tuple_view_adaptor<reverse_view>();

} // namespace upd::tuple_views

template<upd::tuple_like2 Base>
struct upd::tuple_view_for<upd::tuple_views::reverse_view<Base>> {
  using base_type = Base;

  constexpr static auto size = tuple_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return upd::get<size - I - 1>(UPD_FWD(view).base);
  }
};
