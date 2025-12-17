#pragma once

#include <cstddef>
#include <ranges>
#include <utility>

#include "../constexpr.hpp"
#include "../get.hpp"
#include "../tuple/tuple_size.hpp"
#include "../upd.hpp"
#include "../variadic/intersect.hpp"
#include "entry.hpp"
#include "record_like.hpp"
#include "record_view.hpp"
#include "tags_of.hpp"
#include "view.hpp"

namespace upd::record_views {

template<record_like Lhs, record_like Rhs>
struct zip_view {
  Lhs lhs;
  Rhs rhs;
};

template<record_like Lhs, record_like Rhs>
zip_view(Lhs &&, Rhs &&) -> zip_view<Lhs, Rhs>;

constexpr auto zip = []<record_like Lhs, record_like Rhs>(Lhs &&lhs, Rhs &&rhs) {
  auto ensure_view = [](auto &&rec) { return view_t{UPD_FWD(rec)}; };
  return zip_view{ensure_view(UPD_FWD(lhs)), ensure_view(UPD_FWD(rhs))};
};

} // namespace upd::record_views

template<upd::record_like Lhs, upd::record_like Rhs>
struct upd::record_view_for<upd::record_views::zip_view<Lhs, Rhs>> {
  using lhs_type = Lhs;
  using rhs_type = Rhs;

  constexpr static auto tags = intersect(tags_of_v<Lhs>, tags_of_v<Rhs>);
  constexpr static auto size = tuple_size_v<decltype(tags)>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) {
    constexpr auto tag = get<I>(tags).value;

    using lhs_value_type = decltype(get<tag>(UPD_FWD(view).lhs));
    using rhs_value_type = decltype(get<tag>(UPD_FWD(view).rhs));
    using value_type = std::pair<lhs_value_type, rhs_value_type>;
    return entry{expr<tag>, value_type{get<tag>(UPD_FWD(view).lhs), get<tag>(UPD_FWD(view).rhs)}};
  }
};
