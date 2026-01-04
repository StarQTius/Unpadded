#pragma once

#include <cstddef>

#include "../constexpr.hpp"
#include "../get.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "tuple_like.hpp"
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
