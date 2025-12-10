#pragma once

#include <tuple>

#include "../constexpr.hpp"
#include "../record/lite_record.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"

namespace upd::variadic {

template<auto X, metavalue... Xs>
constexpr auto position_of(std::tuple<Xs...>) noexcept(release) {
  auto lut = UPD_WITH_SEQUENCE(Is, sizeof...(Xs)) {
    return lite_record{lite_record_node{expr<Xs::value>, expr<Is>}...};
  };

  if constexpr (lut.has_tag(expr<X>)) {
    return lut.get_by_tag(expr<X>);
  } else {
    return expr<sizeof...(Xs)>;
  }
}

} // namespace upd::variadic
