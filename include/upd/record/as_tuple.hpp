#pragma once

#include <array>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <utility>
#include <variant>

#include "../tuple/tuple_like.hpp"
#include "../tuple/tuple_view.hpp"
#include "../upd.hpp"
#include "entry.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

using upd::get;

template<record_like Base>
struct as_tuple_view {
  Base base;
};

template<record_like Base>
as_tuple_view(Base &&) -> as_tuple_view<Base>;

constexpr auto as_tuple = record_view_adaptor<as_tuple_view>();

} // namespace upd::record_views

template<upd::record_like Base>
struct upd::tuple_view_for<upd::record_views::as_tuple_view<Base>> {
  using base_type = Base;

  constexpr static auto size = record_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    constexpr auto tag = record_tag_v<I, Base>;
    using type = decltype(upd::get<tag>(UPD_FWD(view).base));
    return entry<tag, type>{upd::get<tag>(UPD_FWD(view).base)};
  }
};
