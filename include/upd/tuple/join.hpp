#pragma once

#include <cstddef>

#include "../get.hpp"
#include "../upd.hpp"
#include "../variadic/nested_indices.hpp"
#include "../with_sequence.hpp"
#include "nested_tuple.hpp"
#include "tuple_element.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<nested_tuple2 Base>
struct join_view {
  Base base;
};

template<nested_tuple2 Base>
join_view(Base &&) -> join_view<Base>;

constexpr auto join = tuple_view_adaptor<join_view>();

} // namespace upd::tuple_views

template<upd::nested_tuple2 Base>
struct upd::tuple_view_for<upd::tuple_views::join_view<Base>> {
  using base_type = Base;

  constexpr static auto size = UPD_WITH_SEQUENCE(Is, tuple_size_v<Base>) {
    return (tuple_size_v<tuple_element_t<Is, Base>> + ... + 0zu);
  };

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    constexpr auto nested_indices = UPD_WITH_SEQUENCE(Is, tuple_size_v<Base>) {
      return nested_indices_v<tuple_size_v<tuple_element_t<Is, Base>>...>;
    };

    constexpr auto ni = nested_indices[I];
    constexpr auto i = ni.second;
    constexpr auto j = ni.first;

    return upd::get<j>(upd::get<i>(UPD_FWD(view).base));
  }
};
