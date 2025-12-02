#pragma once

#include "../functional.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "get_ith_entry.hpp"
#include "record_like.hpp"
#include "record_size.hpp"

namespace upd::record_views {

constexpr auto apply = []<record_like Record>(auto &&f, Record &&rec) {
  return UPD_WITH_SEQUENCE(Is, record_size_v<Record>, &) {
    return UPD_INVOKE(UPD_FWD(f), get_ith_entry<Is>(UPD_FWD(rec))...);
  };
};

} // namespace upd::record_views
