#pragma once

#include "upd.hpp"

namespace upd {

template<typename T, template<auto, typename...> typename TT>
[[nodiscard]] constexpr auto is_instance_of() noexcept(release) -> bool {
  auto checker = []<auto V, typename... Ts>(const TT<V, Ts...> &) {};
  return requires(const T &x) { checker(x); };
}

} // namespace upd
