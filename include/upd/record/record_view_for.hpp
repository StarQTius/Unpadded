#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "../constexpr.hpp"
#include "../implementation_of.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "concepts.hpp"
#include "lite_record.hpp"

namespace upd {

template<typename>
struct record_view_for; // IWYU pragma: keep

template<auto Tag, typename View>
  requires upd::implementation_of<View, upd::record_view_for>
[[nodiscard]] constexpr auto get(View &&view) -> decltype(auto) {
  using view_type = std::remove_cvref_t<View>;

  constexpr auto i = UPD_WITH_SEQUENCE(Is, upd::record_size<view_type>::value) {
    auto lut = upd::lite_record{upd::lite_record_node{upd::expr<upd::record_tag<Is, view_type>::value>, Is}...};
    return get<Tag>(lut);
  };

  return upd::record_view_for<std::remove_cvref_t<View>>::template get_ith<i>(UPD_FWD(view)).value;
}

} // namespace upd

template<auto Tag, typename View>
  requires upd::implementation_of<View, upd::record_view_for>
struct upd::record_element<Tag, View> {
  using type = decltype(get<Tag>(std::declval<View>()));
};

template<std::size_t I, typename View>
  requires upd::implementation_of<View, upd::record_view_for>
struct upd::record_tag<I, View> {
  constexpr static auto value =
      decltype(upd::record_view_for<std::remove_cvref_t<View>>::template get_ith<I>(std::declval<View>()))::identifier;
};

template<typename View>
  requires upd::implementation_of<View, upd::record_view_for>
struct upd::record_size<View> {
  constexpr static auto value = upd::record_view_for<std::remove_cvref_t<View>>::size;
};
