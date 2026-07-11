#pragma once

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <utility>

#include "../upd.hpp"
#include "../utility/get.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "view.hpp"

namespace upd::tuple_views {

template<tuple_like2 Lhs, tuple_like2 Rhs>
struct zip_view {
  Lhs lhs;
  Rhs rhs;
};

template<tuple_like2 Lhs, tuple_like2 Rhs>
zip_view(Lhs &&, Rhs &&) -> zip_view<Lhs, Rhs>;

constexpr auto zip = []<tuple_like2 Lhs, tuple_like2 Rhs>(Lhs &&lhs,
                                                          Rhs &&rhs) {
  auto ensure_view = [](auto &&rec) { return view_t{UPD_FWD(rec)}; };
  return zip_view{ensure_view(UPD_FWD(lhs)), ensure_view(UPD_FWD(rhs))};
};

} // namespace upd::tuple_views

template<upd::tuple_like2 Lhs, upd::tuple_like2 Rhs>
struct upd::tuple_view_for<upd::tuple_views::zip_view<Lhs, Rhs>> {
  using lhs_type = Lhs;
  using rhs_type = Rhs;

  constexpr static auto size = std::min(tuple_size_v<Lhs>, tuple_size_v<Rhs>);

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) {
    using lhs_value_type = decltype(upd::get<I>(UPD_FWD(view).lhs));
    using rhs_value_type = decltype(upd::get<I>(UPD_FWD(view).rhs));
    using value_type = std::pair<lhs_value_type, rhs_value_type>;
    return value_type{upd::get<I>(UPD_FWD(view).lhs),
                      upd::get<I>(UPD_FWD(view).rhs)};
  }
};
