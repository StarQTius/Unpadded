#pragma once

#include <cstddef>
#include <utility>

#include "constexpr.hpp"
#include "lite_tuple.hpp"
#include "tuple_impl.hpp"
#include "type_traits.hpp"
#include "typelist.hpp"
#include "upd.hpp"

namespace upd {

template<auto... Vs>
class constlist : public tuple_implementation<constlist<Vs...>> {
  template<std::size_t, typename Tuple>
  friend constexpr auto get(Tuple &&) noexcept(release) -> auto &&;

  constexpr static auto leaves = detail::leaves<std::make_index_sequence<sizeof...(Vs)>, auto_constant<Vs>...>{};

public:
  template<typename... Ts, typename... Args>
  [[nodiscard]] constexpr static auto make_tuple(typelist<Ts...>, Args &&...xs) {
    return tuple<Ts...>{std::in_place, UPD_FWD(xs)...};
  }

  template<typename... Us>
  [[nodiscard]] constexpr static auto make_typelist(typelist<Us...>) noexcept(release) {
    return typelist<Us...>{};
  }

  template<auto... Values>
  [[nodiscard]] constexpr static auto make_constlist(constlist<Values...>) noexcept(release) {
    return constlist<Values...>{};
  }

  constexpr constlist() noexcept(release) = default;

  template<metavalue... Metas>
  constexpr explicit constlist(Metas...) noexcept(release) {}

  template<std::size_t I, typename = void>
  [[nodiscard]] constexpr static auto get() noexcept(release) -> const auto & {
    const auto &retval = leaves.at(expr<I>);

    return retval;
  }
};

template<metavalue... Metas>
explicit constlist(Metas...) -> constlist<Metas::value...>;

template<std::size_t I, auto... Vs>
[[nodiscard]] constexpr auto get(constlist<Vs...>) {
  return constlist<Vs...>::template get<I>();
}

} // namespace upd

template<auto... Values>
struct std::tuple_size<upd::constlist<Values...>> {
  constexpr static auto value = sizeof...(Values);
};

template<std::size_t I, auto... Values>
struct std::tuple_element<I, upd::constlist<Values...>> {
  using type = decltype(auto{upd::detail::lite_tuple<upd::auto_constant<Values>...>{}.at(upd::expr<I>)}) const &;
};
