#pragma once

#include "../functional.hpp"
#include "../get.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd::tuple_views {

constexpr auto for_each = []<tuple_like2 Tuple>(Tuple &&t, auto &&f) -> void {
  UPD_WITH_SEQUENCE(Is, tuple_size_v<Tuple>, &) { ((void)UPD_INVOKE(f, get<Is>(UPD_FWD(t))), ...); };
};

} // namespace upd::tuple_views
