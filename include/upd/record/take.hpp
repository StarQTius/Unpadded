#pragma once

#include <cstddef>
#include <utility>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/type_traits.hpp"
#include "entry.hpp"
#include "find.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<record_like Base, metavalue Metatag>
struct take_until_view {
  Base base;

  explicit constexpr take_until_view(Base b, Metatag) : base{UPD_FWD(b)} {}
};

template<auto Tag>
constexpr auto take_until = record_view_adaptor<take_until_view>(expr<Tag>);

} // namespace upd::record_views

template<upd::record_like Base, upd::metavalue Metatag>
struct upd::record_view_for<upd::record_views::take_until_view<Base, Metatag>> {
  using base_type = Base;

  constexpr static auto size = decltype(record_views::find_tag<Metatag::value>(
      std::declval<Base>()))::value;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) -> decltype(auto) {
    constexpr auto tag = record_tag_v<I, Base>;
    using value_type = decltype(upd::get_ith<I>(UPD_FWD(view).base));
    return entry<tag, value_type>{upd::get_ith<I>(UPD_FWD(view).base)};
  }
};
