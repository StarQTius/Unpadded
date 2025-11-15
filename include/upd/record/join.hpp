#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <utility>

#include "../constexpr.hpp"
#include "../functional.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "entry.hpp"
#include "ith_record_element.hpp"
#include "nested_record.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<nested_record Base, typename Joiner>
struct join_view {
  Base base;
  Joiner joiner;
};

template<nested_record Base, typename Joiner>
join_view(Base &&, Joiner) -> join_view<Base, Joiner>;

constexpr auto join = record_view_adaptor<join_view>;

} // namespace upd::record_views

template<upd::nested_record Base, typename Joiner>
struct upd::record_view_for<upd::record_views::join_view<Base, Joiner>> {
  using base_type = Base;

  constexpr static auto size = UPD_WITH_SEQUENCE(Is, record_size_v<Base>) {
    return (record_size_v<ith_record_element_t<Is, Base>> + ... + 0zu);
  };

  constexpr static auto nested_indices = UPD_WITH_SEQUENCE(Is, record_size_v<Base>) {
    namespace stdr = std::ranges;
    namespace stdv = std::views;

    using nested_index_type = std::pair<std::size_t, std::size_t>;

    auto retval = std::array<nested_index_type, size>{};
    auto subsizes = std::array{record_size_v<ith_record_element_t<Is, Base>>...};
    auto i = 0zu;
    auto it = retval.begin();
    for (auto ss : subsizes) {
      it = stdr::copy(stdv::repeat(i) | stdv::take(ss) | stdv::enumerate, it).out;
      ++i;
    }

    return retval;
  };

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) {
    constexpr auto ni = nested_indices[I];
    constexpr auto i = ni.second;
    constexpr auto j = ni.first;

    using subrec_type = ith_record_element_t<i, Base>;
    constexpr auto tag = record_tag_v<i, Base>;
    constexpr auto subtag = record_tag_v<j, subrec_type>;
    constexpr auto joined_tag = UPD_INVOKE(view.joiner, tag, subtag);

    using value_type = decltype(get<subtag>(get<tag>(UPD_FWD(view).base)));
    return entry<joined_tag, value_type>{get<subtag>(get<tag>(UPD_FWD(view).base))};
  }
};
