#pragma once

#include "../functional.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "clean.hpp"
#include "transform.hpp"
#include "view_chain.hpp"

namespace upd::record_views {

constexpr auto filter = [](auto &&pred) {
  struct cleanme {};
  auto f = [pred = UPD_FWD(pred)](auto k, auto &&v) -> decltype(auto) {
    constexpr auto keep_it = UPD_INVOKE(pred, k, typebox<decltype(v) &&>{});
    if constexpr (keep_it) {
      return UPD_FWD(v);
    } else {
      return cleanme{};
    }
  };

  return view_chain{transform(UPD_FWD(f)), clean<cleanme>};
};

} // namespace upd::record_views
