#pragma once

#include <utility>

#define UPD_INVOKE(INVOCABLE, ...) ((INVOCABLE)(__VA_ARGS__))

namespace upd {

template<typename T, typename... Args>
concept invocable = requires(T &&x, Args &&...args) { UPD_INVOKE(UPD_FWD(x), UPD_FWD(args)...); };

template<typename F, typename... Args>
struct invoke_result {
  using type = decltype(UPD_INVOKE(std::declval<F>(), std::declval<Args>()...));
};

template<typename F, typename... Args>
using invoke_result_t = typename invoke_result<F, Args...>::type;

constexpr inline auto equal_to = [](auto &&lhs, auto &&rhs) { return UPD_FWD(lhs) == UPD_FWD(rhs); };

constexpr inline auto plus = [](auto &&lhs, auto &&rhs) { return UPD_FWD(lhs) + UPD_FWD(rhs); };

constexpr inline auto invoke = [](auto &&f, auto &&...args) { return UPD_FWD(f)(UPD_FWD(args)...); };

} // namespace upd
