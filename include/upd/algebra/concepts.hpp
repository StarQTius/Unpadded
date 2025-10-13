#pragma once

#include <type_traits>

#include "../record/babelian_lite_record.hpp"
#include "../record/concepts.hpp"
#include "../upd.hpp"

namespace upd::algebra {

template<typename T, record_like Lets>
  requires std::is_arithmetic_v<T>
[[nodiscard]] constexpr auto substitute(T value, const Lets &) noexcept(release) -> T {
  return value;
}

template<typename Expr>
concept expression = requires(Expr expr) { substitute(expr, babelian_lite_record{0}); };

} // namespace upd::algebra
