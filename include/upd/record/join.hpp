#pragma once

#include <cstddef>

#include "../upd.hpp"
#include "../utility/get.hpp"
#include "../utility/with_sequence.hpp"
#include "../variadic/nested_indices.hpp"
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

constexpr auto join = [](auto &&joiner) {
  auto j = [joiner = UPD_FWD(joiner)]<auto Tag, auto Subtag> {
    return UPD_INVOKE_TEMPLATE(joiner, (Tag, Subtag));
  };

  return record_view_adaptor<join_view>(UPD_FWD(j));
};

} // namespace upd::record_views

template<upd::nested_record Base, typename Joiner>
struct upd::record_view_for<upd::record_views::join_view<Base, Joiner>> {
  using base_type = Base;

  constexpr static auto size = UPD_WITH_SEQUENCE(Is, record_size_v<Base>) {
    return (record_size_v<ith_record_element_t<Is, Base>> + ... + 0zu);
  };

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) {
    constexpr auto nested_indices = UPD_WITH_SEQUENCE(Is, record_size_v<Base>) {
      return nested_indices_v<record_size_v<ith_record_element_t<Is, Base>>...>;
    };

    constexpr auto ni = nested_indices[I];
    constexpr auto i = ni.second;
    constexpr auto j = ni.first;

    using subrec_type = ith_record_element_t<i, Base>;
    constexpr auto tag = record_tag_v<i, Base>;
    constexpr auto subtag = record_tag_v<j, subrec_type>;
    constexpr auto joined_tag = UPD_INVOKE_TEMPLATE(view.joiner, (tag, subtag));

    using value_type = decltype(get<subtag>(get<tag>(UPD_FWD(view).base)));
    return entry<joined_tag, value_type>{
        get<subtag>(get<tag>(UPD_FWD(view).base))};
  }
};
