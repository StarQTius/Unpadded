#pragma once

#include <array>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <utility>
#include <variant>

#include "../named_value.hpp"
#include "../upd.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"

namespace upd::tuple_views {

template<tuple_like2 Base>
struct view_t {
  Base &&base;
};

template<tuple_like2 Base>
view_t(Base &&) -> view_t<Base &&>;

} // namespace upd::tuple_views

template<upd::tuple_like2 Base>
struct upd::tuple_view_for<upd::tuple_views::view_t<Base>> {
  using base_type = Base &&;

  constexpr static auto size = tuple_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return upd::get<I>(UPD_FWD(view.base));
  }
};
