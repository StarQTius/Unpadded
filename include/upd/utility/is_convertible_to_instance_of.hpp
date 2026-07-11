#pragma once

#include "../upd.hpp"

namespace upd {

template<typename T, template<auto, typename...> typename TT>
[[nodiscard]] constexpr auto
is_convertible_to_instance_of() noexcept(release) -> bool {
  return requires(const T &x) { TT{x}; };
}

} // namespace upd
