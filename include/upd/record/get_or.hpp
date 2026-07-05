#pragma once

#include "../upd.hpp"
#include "../utility/get.hpp"
#include "has_tag.hpp"
#include "record_like.hpp"

namespace upd {

template<auto Tag, record_like Record, typename T>
[[nodiscard]] constexpr auto get_or(Record &&rec, T &&x) -> decltype(auto) {
  if constexpr (has_tag<Tag>(rec)) {
    return get<Tag>(UPD_FWD(rec));
  } else {
    return UPD_FWD(x);
  }
}

} // namespace upd
