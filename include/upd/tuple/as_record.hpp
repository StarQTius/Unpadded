#pragma once

#include <cstddef>

#include "../record/entry.hpp"
#include "../record/record_view.hpp"
#include "../upd.hpp"
#include "../utility/get.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

using upd::get;

template<tuple_like2 Base>
struct as_record_view {
  Base base;
};

template<tuple_like2 Base>
as_record_view(Base &&) -> as_record_view<Base>;

constexpr auto as_record = tuple_view_adaptor<as_record_view>();

} // namespace upd::tuple_views

template<upd::tuple_like2 Base>
struct upd::record_view_for<upd::tuple_views::as_record_view<Base>> {
  using base_type = Base;

  constexpr static auto size = tuple_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) {
    using entry_type = tuple_element_t<I, Base>;
    constexpr auto identifier = entry_type::identifier;
    using value_type = decltype(get<I>(UPD_FWD(view).base).value);
    return entry<identifier, value_type>{get<I>(UPD_FWD(view).base).value};
  }
};
