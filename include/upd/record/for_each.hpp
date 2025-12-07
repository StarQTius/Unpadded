#pragma once

#include "../constexpr.hpp"
#include "../functional.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd::record_views {

constexpr auto for_each = []<record_like Record>(Record &&rec, auto &&f) -> void {
  UPD_WITH_SEQUENCE(Is, record_size_v<Record>, &) {
    ((void)UPD_INVOKE(f, expr<record_tag_v<Is, Record>>, get_ith<Is>(UPD_FWD(rec))), ...);
  };
};

} // namespace upd::record_views
