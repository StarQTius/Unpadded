#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "constexpr.hpp"
#include "functional.hpp"
#include "lite_tuple.hpp"
#include "ref.hpp"
#include "template_traits.hpp"
#include "tuple_impl.hpp"
#include "type_traits.hpp"
#include "upd.hpp"

namespace upd {

template<typename... Ts>
class typelist : public tuple_implementation<typelist<Ts...>> {
  using leaves = detail::leaves<std::index_sequence_for<Ts...>, typebox<Ts>...>;

public:
  template<typename... Us, typename... Args>
  [[nodiscard]] constexpr static auto make_tuple(typelist<Us...>, Args &&...xs) {
    return tuple<Us...>{std::in_place, UPD_FWD(xs)...};
  }

  template<typename... Us>
  [[nodiscard]] constexpr static auto make_typelist(typelist<Us...>) noexcept(release) {
    return typelist<Us...>{};
  }

  template<auto... Values>
  [[nodiscard]] constexpr static auto make_constlist(constlist<Values...>) noexcept(release) {
    return constlist<Values...>{};
  }

  constexpr typelist() noexcept(release) = default;

  template<metatype... Metas>
  constexpr explicit typelist(Metas...) noexcept(release) {}

  template<std::size_t I, typename Self>
  [[nodiscard]] constexpr auto get(this Self &&self) noexcept(release) -> auto && {
    decltype(auto) retval = UPD_FWD(self).m_leaves.at(expr<I>);

    return UPD_FWD(retval);
  }

  template<typename F>
  [[nodiscard]] constexpr static auto metatransform(F &&) noexcept(release) -> typelist<invoke_result_t<F &, Ts>...> {
    return {};
  }

  template<template<typename...> typename TT>
  [[nodiscard]] constexpr static auto metaapply() noexcept(release) -> TT<Ts...>;

private:
  leaves m_leaves;
};

template<typename... Ts>
  requires((sizeof...(Ts) > 1 || sizeof...(Ts) == 1 && !(tuple_like<Ts> && ...)) &&
           !typelist<Ts...>{}[expr<0>].template satisfies<std::is_same, std::in_place_t>())
tuple(Ts...) -> tuple<typename std::conditional_t<instance_of<Ts, ref>, Ts, std::type_identity<Ts>>::type...>;

template<typename... Ts>
explicit tuple(std::in_place_t, Ts...)
    -> tuple<typename std::conditional_t<instance_of<Ts, ref>, Ts, std::type_identity<Ts>>::type...>;

template<metatype... Metas>
explicit typelist(Metas...) -> typelist<typename Metas::type...>;

} // namespace upd

template<typename... Ts>
struct std::tuple_size<upd::typelist<Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

template<std::size_t I, typename... Ts>
struct std::tuple_element<I, upd::typelist<Ts...>> {
  using type = decltype(auto{upd::detail::lite_tuple<upd::typebox<Ts>...>{}.at(upd::expr<I>)});
};
