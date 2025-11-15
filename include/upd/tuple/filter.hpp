#pragma once

#include "../functional.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "chain.hpp"
#include "clean.hpp"
#include "transform.hpp"

namespace upd::tuple_views {

constexpr auto filter = [](auto &&pred) {
  struct cleanme {};
  auto f = [pred = UPD_FWD(pred)](auto &&v) -> decltype(auto) {
    constexpr auto keep_it = UPD_INVOKE(pred, typebox<decltype(v) &&>{});
    if constexpr (keep_it) {
      return UPD_FWD(v);
    } else {
      return cleanme{};
    }
  };

  return chain{transform(UPD_FWD(f)), clean<cleanme>};
};

} // namespace upd::tuple_views
