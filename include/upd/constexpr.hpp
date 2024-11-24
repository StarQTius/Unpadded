#pragma once

#include <concepts>
#include <compare>
#include <functional>
#include "upd.hpp"

namespace upd {

template<auto>
struct auto_constant;

template<typename T>
concept auto_constant_instance = requires(T x) {
  { auto_constant{x} } -> std::common_reference_with<T>;
};

template<auto Expr>
constexpr auto expr = auto_constant<Expr>{};

template<auto Value>
struct auto_constant {
  using type = auto_constant<Value>;
  using value_type = std::remove_cvref_t<decltype(Value)>;

  constexpr static auto value = Value;

  template<typename T> requires std::convertible_to<value_type, T>
  [[nodiscard]] constexpr operator T() const {
    return static_cast<T>(value);
  }

  template<auto... Args> requires std::invocable<value_type, decltype(Args)...>
  [[nodiscard]] constexpr static auto operator()(auto_constant<Args>...) noexcept(release) {
    return expr<std::invoke(value, Args...)>;
  }

  template<typename... Args> requires std::invocable<value_type, Args...>
  [[nodiscard]] constexpr static auto operator()(Args && ... args) -> decltype(auto) {
    return std::invoke(value, UPD_FWD(args)...);
  }

  template<auto... Args> requires requires { value[Args...]; }
  [[nodiscard]] constexpr static auto operator[](auto_constant<Args>...) noexcept(release) {
    return expr<value[Args...]>;
  }

  template<typename... Args> requires requires { value[std::declval<Args>()...]; }
  [[nodiscard]] constexpr static auto operator[](Args && ... args) -> decltype(auto) {
    return value[UPD_FWD(args)...];
  }
};

template<auto LhsValue, auto RhsValue> requires requires { LhsValue == RhsValue; }
[[nodiscard]] constexpr auto operator==(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue == RhsValue>;
}

template<auto LhsValue, typename Rhs>
requires requires { LhsValue == std::declval<Rhs>(); }
[[nodiscard]] consteval auto operator==(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue == UPD_FWD(rhs);
}

template<auto LhsValue, auto RhsValue>
requires requires { LhsValue <=> RhsValue; }
[[nodiscard]] constexpr auto operator<=>(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue <=> RhsValue>;
}

template<auto LhsValue, typename Rhs>
requires requires { LhsValue <=> std::declval<Rhs>(); }
[[nodiscard]] consteval auto operator<=>(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue <=> UPD_FWD(rhs);
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue + RhsValue; }
[[nodiscard]] constexpr auto operator+(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue + RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue + std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator+(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue + UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() + RhsValue; }
[[nodiscard]] constexpr auto operator+(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) + RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue - RhsValue; }
[[nodiscard]] constexpr auto operator-(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue - RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue - std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator-(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue - UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() - RhsValue; }
[[nodiscard]] constexpr auto operator-(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) - RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue * RhsValue; }
[[nodiscard]] constexpr auto operator*(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue * RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue * std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator*(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue * UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() * RhsValue; }
[[nodiscard]] constexpr auto operator*(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) * RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue / RhsValue; }
[[nodiscard]] constexpr auto operator/(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue / RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue / std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator/(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue / UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() / RhsValue; }
[[nodiscard]] constexpr auto operator/(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) / RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue % RhsValue; }
[[nodiscard]] constexpr auto operator%(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue % RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue % std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator%(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue % UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() % RhsValue; }
[[nodiscard]] constexpr auto operator%(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) % RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue ^ RhsValue; }
[[nodiscard]] constexpr auto operator^(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue ^ RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue ^ std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator^(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue ^ UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() ^ RhsValue; }
[[nodiscard]] constexpr auto operator^(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) ^ RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue & RhsValue; }
[[nodiscard]] constexpr auto operator&(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue & RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue & std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator&(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue & UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() & RhsValue; }
[[nodiscard]] constexpr auto operator&(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) & RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue | RhsValue; }
[[nodiscard]] constexpr auto operator|(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue | RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue | std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator|(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue | UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() | RhsValue; }
[[nodiscard]] constexpr auto operator|(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) | RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue << RhsValue; }
[[nodiscard]] constexpr auto operator<<(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue << RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue << std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator<<(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue << UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() << RhsValue; }
[[nodiscard]] constexpr auto operator<<(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) << RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue >> RhsValue; }
[[nodiscard]] constexpr auto operator>>(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<(LhsValue >> RhsValue)>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue >> std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator>>(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue >> UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() >> RhsValue; }
[[nodiscard]] constexpr auto operator>>(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) >> RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue && RhsValue; }
[[nodiscard]] constexpr auto operator&&(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue && RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue && std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator&&(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue && UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() && RhsValue; }
[[nodiscard]] constexpr auto operator&&(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) && RhsValue;
}

template<auto LhsValue, auto RhsValue> requires requires { LhsValue || RhsValue; }
[[nodiscard]] constexpr auto operator||(auto_constant<LhsValue>, auto_constant<RhsValue>) noexcept(release) {
  return expr<LhsValue || RhsValue>;
}

template<auto LhsValue, typename Rhs> requires requires { LhsValue || std::declval<Rhs>(); }
[[nodiscard]] constexpr auto operator||(auto_constant<LhsValue>, Rhs &&rhs) -> decltype(auto) {
  return LhsValue || UPD_FWD(rhs);
}

template<typename Lhs, auto RhsValue> requires requires { std::declval<Lhs>() || RhsValue; }
[[nodiscard]] constexpr auto operator||(Lhs &&lhs, auto_constant<RhsValue>) -> decltype(auto) {
  return UPD_FWD(lhs) || RhsValue;
}

template<auto Value> requires requires { +Value; }
[[nodiscard]] constexpr auto operator+(auto_constant<Value>) noexcept(release) {
  return expr<+Value>;
}

template<auto Value> requires requires { -Value; }
[[nodiscard]] constexpr auto operator-(auto_constant<Value>) noexcept(release) {
  return expr<-Value>;
}

template<auto Value> requires requires { ~Value; }
[[nodiscard]] constexpr auto operator~(auto_constant<Value>) noexcept(release) {
  return expr<~Value>;
}

template<auto Value> requires requires { !Value; }
[[nodiscard]] constexpr auto operator!(auto_constant<Value>) noexcept(release) {
  return expr<!Value>;
}

} // namespace upd
