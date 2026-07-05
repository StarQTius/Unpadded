#pragma once

#include <concepts>
#include <utility>

#include "../upd.hpp"

namespace upd {

template<std::integral Lhs, std::integral Rhs>
[[nodiscard]] constexpr auto safe_not_equal(const Lhs &lhs, const Rhs &rhs) noexcept(release) -> bool {
  return std::cmp_not_equal(lhs, rhs);
}

template<typename Lhs, typename Rhs>
[[nodiscard]] constexpr auto safe_not_equal(const Lhs &lhs, const Rhs &rhs) -> bool {
  return lhs != rhs;
}

} // namespace upd
