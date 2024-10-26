#pragma once

namespace upd {

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
