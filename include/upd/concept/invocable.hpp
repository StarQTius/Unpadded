#pragma once

#include "../upd.hpp"

namespace upd {

template<typename F, typename... Args>
concept invocable = requires(F &&f, Args &&...args) { UPD_INVOKE(UPD_FWD(f), UPD_FWD(args)...); };

} // namespace upd
