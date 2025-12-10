#pragma once

#include <cstddef>
#include <utility>

#include "constexpr.hpp"
#include "record/lite_record.hpp"
#include "tuple_impl.hpp"
#include "type_traits.hpp"
#include "upd.hpp"

namespace upd::detail {

template<std::size_t I, typename T>
struct leaf {
  [[nodiscard]] constexpr auto at(auto_constant<I>) & noexcept -> T & { return value; }

  [[nodiscard]] constexpr auto at(auto_constant<I>) const & noexcept -> const T & { return value; }

  [[nodiscard]] constexpr auto at(auto_constant<I>) && noexcept -> T && { return UPD_FWD(value); }

  [[nodiscard]] constexpr auto at(auto_constant<I>) const && noexcept -> const T && { return UPD_FWD(value); }

  [[nodiscard]] constexpr static auto typebox_at(auto_constant<I>) noexcept -> typebox<T>;

  [[nodiscard]] constexpr static auto has_type(typebox<T>) noexcept(release) -> bool { return true; }

  T value;
};

template<typename, typename...>
struct leaves;

template<std::size_t... Is, typename... Ts>
struct leaves<std::index_sequence<Is...>, Ts...> : leaf<Is, Ts>... {
  using leaf<Is, Ts>::at...;
  using leaf<Is, Ts>::typebox_at...;
  using leaf<Is, Ts>::has_type...;

  [[nodiscard]] constexpr static auto typebox_at(...) noexcept -> int { return 0; }

  [[nodiscard]] constexpr static auto has_type(...) noexcept(release) -> bool { return false; }

  template<std::size_t I>
  using raw_type = typename decltype(typebox_at(auto_constant<I>{}))::type;

  constexpr static auto size = sizeof...(Ts);

  constexpr leaves() = default;

  template<typename... Us>
  explicit constexpr leaves(std::in_place_t, Us &&...xs) : leaf<Is, Ts>{UPD_FWD(xs)}... {}

  template<tuple_like Tuple>
  explicit constexpr leaves(Tuple &&t) : leaf<Is, Ts>{get<Is>(UPD_FWD(t))}... {}

  [[nodiscard]] constexpr auto at(...) const noexcept -> int { return 0; }
};

template<typename... Ts>
explicit leaves(std::in_place_t, Ts...) -> leaves<Ts...>;

template<typename... Ts>
using lite_tuple = leaves<std::index_sequence_for<Ts...>, Ts...>;

} // namespace upd::detail
