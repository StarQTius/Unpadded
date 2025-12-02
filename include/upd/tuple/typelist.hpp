#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

#include "../detail/fail_unless_discarded.hpp"
#include "tuple_view.hpp"

namespace upd {

template<typename... Ts>
struct typelist2_t {};

template<typename... Ts>
constexpr auto typelist2 = typelist2_t<Ts...>{};

} // namespace upd

template<typename... Ts>
struct upd::tuple_view_for<upd::typelist2_t<Ts...>> {
  constexpr static auto size = sizeof...(Ts);

  template<std::size_t I, typename View>
  [[nodiscard, noreturn]] constexpr static auto get(View) -> std::tuple_element_t<I, std::tuple<Ts...>> {
    detail::fail_unless_discarded("This function cannot be called in evaluated context");
  }
};
