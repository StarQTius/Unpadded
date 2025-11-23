#pragma once

#include <cstddef>

#include "../upd.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"

namespace upd::record_views {

template<record_like Base>
struct view_t {
  Base &&base;
};

template<record_like Base>
view_t(Base &&) -> view_t<Base &&>;

} // namespace upd::record_views

template<upd::record_like Base>
struct upd::record_view_for<upd::record_views::view_t<Base>> {
  using base_type = Base &&;

  constexpr static auto size = record_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) {
    constexpr auto tag = record_tag_v<I, Base>;
    using value_type = decltype(upd::get_ith<I>(UPD_FWD(view.base)));
    return entry<tag, value_type>{upd::get_ith<I>(UPD_FWD(view.base))};
  }
};
