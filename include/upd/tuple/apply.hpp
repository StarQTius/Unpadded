#pragma once

#include "../upd.hpp"
#include "../utility/get.hpp"
#include "../utility/with_sequence.hpp"
#include "instantiate.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "typelist.hpp"

namespace upd::tuple_views {

constexpr auto apply = []<tuple_like2 Tuple, typename F> [[nodiscard]] (
                           Tuple &&t, F &&f) -> decltype(auto) {
  return UPD_WITH_SEQUENCE(Is, tuple_size_v<Tuple>, &) {
    return UPD_INVOKE(UPD_FWD(f), get<Is>(UPD_FWD(t))...);
  };
};

constexpr auto apply_type =
    []<tuple_like2 Tuple, typename F> [[nodiscard]] (Tuple &&t, F &&f)
    -> decltype([]<typename... Ts>(typelist2_t<Ts...>)
                    -> decltype(UPD_INVOKE_TEMPLATE(UPD_FWD(f), (Ts...))) {
}(t | instantiate<typelist2_t>)) {};

} // namespace upd::tuple_views
