#pragma once

#define UPD_INVOKE(INVOCABLE, ...) ((INVOCABLE)(__VA_ARGS__))

namespace upd {

template<typename T, typename... Args>
concept invocable = requires(T &&x, Args &&... args) {
  UPD_INVOKE(UPD_FWD(x), UPD_FWD(args)...);
};

constexpr inline auto equal_to = [](auto &&lhs, auto &&rhs) {
  return UPD_FWD(lhs) == UPD_FWD(rhs);
};

constexpr inline auto plus = [](auto &&lhs, auto &&rhs) {
  return UPD_FWD(lhs) + UPD_FWD(rhs);
};

constexpr inline auto invoke = [](auto &&f, auto && ...args) {
  return UPD_FWD(f)(UPD_FWD(args)...);
};

} // namespace upd
