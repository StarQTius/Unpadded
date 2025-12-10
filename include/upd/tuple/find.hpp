#pragma once

#include <type_traits>

#include "../constexpr.hpp"
#include "../functional.hpp"
#include "../get.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "enumerate.hpp"
#include "filter.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd::tuple_views {

constexpr auto find_if = []<tuple_like2 Tuple>(Tuple &&t, auto &&pred) {
  auto p = [pred]<typename IAndTypebox>(typebox<IAndTypebox>) constexpr {
    using value_type = typename std::remove_cvref_t<IAndTypebox>::second_type;
    return UPD_INVOKE(pred, typebox<value_type>{});
  };

  auto vw = UPD_FWD(t) | enumerate | filter(p);
  if constexpr (tuple_size_v<decltype(vw)> > 0) {
    return get<0>(vw).first;
  } else {
    return expr<tuple_size_v<Tuple>>;
  }
};

} // namespace upd::tuple_views
