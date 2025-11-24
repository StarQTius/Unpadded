#pragma once

#include <algorithm>
#include <cstddef>
#include <ranges>
#include <utility>

#include "../constexpr.hpp"
#include "../functional.hpp"
#include "../upd.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
#include "view.hpp"

namespace upd::record_views {

template<record_like Lhs, record_like Rhs, typename Zipper>
struct zip_view {
  Lhs lhs;
  Rhs rhs;
  Zipper zipper;
};

template<record_like Lhs, record_like Rhs, typename Zipper>
zip_view(Lhs &&, Rhs &&, Zipper) -> zip_view<Lhs, Rhs, Zipper>;

constexpr auto zip = []<record_like Lhs, record_like Rhs, typename Zipper>(Lhs &&lhs, Rhs &&rhs, Zipper &&zipper) {
  auto ensure_view = [](auto &&rec) { return view_t{UPD_FWD(rec)}; };
  return zip_view{ensure_view(UPD_FWD(lhs)), ensure_view(UPD_FWD(rhs)), UPD_FWD(zipper)};
};

} // namespace upd::record_views

template<upd::record_like Lhs, upd::record_like Rhs, typename Zipper>
struct upd::record_view_for<upd::record_views::zip_view<Lhs, Rhs, Zipper>> {
  using lhs_type = Lhs;
  using rhs_type = Rhs;

  constexpr static auto size = std::min(record_size_v<Lhs>, record_size_v<Rhs>);

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) {
    using lhs_value_type = decltype(upd::get_ith<I>(UPD_FWD(view).lhs));
    using rhs_value_type = decltype(upd::get_ith<I>(UPD_FWD(view).rhs));
    using value_type = std::pair<lhs_value_type, rhs_value_type>;
    return entry{expr<UPD_INVOKE(view.zipper, record_tag_v<I, Lhs>, record_tag_v<I, Rhs>)>,
                 value_type{upd::get_ith<I>(UPD_FWD(view).lhs), upd::get_ith<I>(UPD_FWD(view).rhs)}};
  }
};
