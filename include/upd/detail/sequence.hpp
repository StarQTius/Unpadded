#pragma once

#include <type_traits>

namespace upd::detail {

template<typename F, std::size_t... Is>
[[nodiscard]] constexpr auto apply_on_index_sequence(F &&f, std::index_sequence<Is...>) -> decltype(auto) {
  return std::invoke(UPD_FWD(f), std::integral_constant<std::size_t, Is>{}...);
}

} // namespace upd::detail