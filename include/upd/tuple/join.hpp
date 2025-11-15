#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <utility>

#include "../constexpr.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "nested_tuple.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
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

  constexpr static auto nested_indices = UPD_WITH_SEQUENCE(Is, tuple_size_v<Base>) {
    namespace stdr = std::ranges;
    namespace stdv = std::views;

    using nested_index_type = std::pair<std::size_t, std::size_t>;

    auto retval = std::array<nested_index_type, size>{};
    auto subsizes = std::array{tuple_size_v<tuple_element_t<Is, Base>>...};
    auto i = 0zu;
    auto it = retval.begin();
    for (auto ss : subsizes) {
      it = stdr::copy(stdv::repeat(i) | stdv::take(ss) | stdv::enumerate, it).out;
      ++i;
    }

    return retval;
  };

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    constexpr auto ni = nested_indices[I];
    constexpr auto i = ni.second;
    constexpr auto j = ni.first;

    return upd::get<j>(upd::get<i>(UPD_FWD(view).base));
  }
};
