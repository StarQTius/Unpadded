#pragma once

#include <cstddef>

#include "record_like.hpp"
#include "record_size.hpp"
#include "record_view.hpp"
#include "record_view_adaptor.hpp"
#include "tags_of.hpp"

namespace upd::record_views {

template<record_like Base>
struct tags_view {
  Base base;
};

template<record_like Base>
tags_view(Base &&) -> tags_view<Base>;

constexpr auto tags = record_view_adaptor<tags_view>();

} // namespace upd::record_views

template<upd::record_like Base>
struct upd::record_view_for<upd::record_views::tags_view<Base>> {
  using base_type = Base;

  constexpr static auto size = record_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&) -> decltype(auto) {
    return get<I>(tags_of_v<Base>);
  }
};
