#pragma once

#include "always_false.hpp"

namespace upd::detail {

constexpr inline auto fail_on_invoke = [](const auto &...) {
  static_assert(UPD_ALWAYS_FALSE, "This invocable should not have been invoked");
};

} // namespace upd::detail
