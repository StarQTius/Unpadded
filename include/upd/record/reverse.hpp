#pragma once

#include <cstddef>

#include "../upd.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<record_like Base>
struct reverse_view {
  Base base;
};

template<record_like Base>
reverse_view(Base &&) -> reverse_view<Base>;

constexpr auto reverse = record_view_adaptor<reverse_view>();

} // namespace upd::record_views

template<upd::record_like Base>
struct upd::record_view_for<upd::record_views::reverse_view<Base>> {
  using base_type = Base;

  constexpr static auto size = record_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) -> decltype(auto) {
    using value_type = decltype(upd::get_ith<size - I - 1>(UPD_FWD(view).base));
    return entry<record_tag_v<size - I - 1, Base>, value_type>{upd::get_ith<size - I - 1>(UPD_FWD(view).base)};
  }
};
