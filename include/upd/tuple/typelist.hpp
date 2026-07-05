#pragma once

#include <cstddef>
#include <tuple>
#include <variant>

#include "../detail/fail_unless_discarded.hpp"
#include "../upd.hpp"
#include "../utility/collector_of.hpp"
#include "../utility/with_sequence.hpp"
#include "../variadic/template_box.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd {

template<typename... Ts>
struct typelist2_t {};

template<typename... Ts>
constexpr auto typelist2 = typelist2_t<Ts...>{};

} // namespace upd

template<typename... Ts>
struct upd::tuple_like_for<upd::typelist2_t<Ts...>> {
  constexpr static auto size = sizeof...(Ts);

  template<std::size_t I>
  using element_type = tuple_element_t<I, std::tuple<Ts...>>;

  template<std::size_t I, typename Typelist>
  [[nodiscard, noreturn]] constexpr static auto get(Typelist &) noexcept(release) -> element_type<I> & {
    detail::fail_unless_discarded("This function cannot be called in evaluated context");
  }

  template<std::size_t I, typename Typelist>
  [[nodiscard, noreturn]] constexpr static auto get(Typelist &&) noexcept(release) -> element_type<I> && {
    detail::fail_unless_discarded("This function cannot be called in evaluated context");
  }

  template<std::size_t I, typename Typelist>
  [[nodiscard, noreturn]] constexpr static auto get(const Typelist &) noexcept(release) -> const element_type<I> & {
    detail::fail_unless_discarded("This function cannot be called in evaluated context");
  }

  template<std::size_t I, typename Typelist>
  [[nodiscard, noreturn]] constexpr static auto get(const Typelist &&) noexcept(release) -> const element_type<I> && {
    detail::fail_unless_discarded("This function cannot be called in evaluated context");
  }
};

template<>
struct upd::collector_for<upd::template_box<upd::typelist2_t>> {
  template<tuple_like2 View>
  [[nodiscard]] constexpr static auto collect(View &&) {
    return UPD_WITH_SEQUENCE(Is, tuple_size_v<View>) { return typelist2<tuple_element_t<Is, View>...>; };
  }
};
