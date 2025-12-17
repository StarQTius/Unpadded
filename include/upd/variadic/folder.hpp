#pragma once

#include "../upd.hpp"

namespace upd::variadic {

template<typename T, typename BinaryOp>
struct folder_t {
  T value;
  BinaryOp oper;
};

template<typename T, typename BinaryOp>
[[nodiscard]] constexpr auto folder(T &&value, BinaryOp &op) {
  return folder_t<T &&, BinaryOp &>{UPD_FWD(value), op};
}

template<typename Lhs, typename Rhs, typename BinaryOp>
[[nodiscard]] constexpr auto operator,(Lhs &&lhs, folder_t<Rhs, BinaryOp> &&fder) -> decltype(auto) {
  return UPD_INVOKE(UPD_FWD(UPD_FWD(fder).oper), UPD_FWD(lhs), UPD_FWD(UPD_FWD(fder).value));
}

template<typename Lhs, typename BinaryOp, typename Rhs>
[[nodiscard]] constexpr auto operator,(folder_t<Lhs, BinaryOp> fder, Rhs &&rhs) -> decltype(auto) {
  return UPD_INVOKE(UPD_FWD(UPD_FWD(fder).oper), UPD_FWD(UPD_FWD(fder).value), UPD_FWD(rhs));
}

} // namespace upd::variadic
