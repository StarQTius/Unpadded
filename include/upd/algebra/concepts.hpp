#pragma once

#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>

#include "../concept/invocable.hpp"
#include "../record/record_like.hpp"
#include "../record/universal_record.hpp"
#include "../upd.hpp"

namespace upd::algebra {

template<typename, auto>
struct depends_on;

template<typename T, record_like Lets>
  requires std::is_scalar_v<T>
[[nodiscard]] constexpr auto substitute(T value, const Lets &) noexcept(release) -> T {
  return value;
}

template<typename T>
  requires std::is_scalar_v<T>
[[nodiscard]] constexpr auto dependencies(T) noexcept(release) -> std::tuple<> {
  return std::tuple{};
}

template<typename T, auto Varname>
  requires std::is_scalar_v<T>
struct depends_on<T, Varname> {
  constexpr static auto value = false;
};

template<typename T, record_like Lets>
  requires std::is_scalar_v<T>
[[nodiscard]] constexpr auto substitute(std::reference_wrapper<T> ref, const Lets &) noexcept(release) {
  return ref;
}

template<typename T, auto Varname>
  requires std::is_scalar_v<T>
struct depends_on<std::reference_wrapper<T>, Varname> {
  constexpr static auto value = false;
};

template<invocable F, record_like Lets>
[[nodiscard]] constexpr auto substitute(const F &f, const Lets &) noexcept(release) {
  return UPD_INVOKE(f);
}

template<invocable F, auto Varname>
struct depends_on<F, Varname> {
  constexpr static auto value = false;
};

template<typename Expr>
concept expression = requires(Expr expr) {
  substitute(expr, universal_record{0});
  typename depends_on<Expr, 0>;
  { depends_on<Expr, 0>::value } -> std::convertible_to<bool>;
};

template<typename Expr, auto Varname>
concept balanceable = expression<Expr> && requires(Expr expr) {
  { balance_on<Varname>(expr, expr) } -> expression;
};

template<expression Expr, auto Varname>
constexpr auto depends_on_v = depends_on<Expr, Varname>::value;

} // namespace upd::algebra
