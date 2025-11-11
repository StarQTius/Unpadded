#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <utility>

#include "../implementation_of.hpp"
#include "../upd.hpp"
#include "../variadic_concept.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd {

template<typename View, std::size_t I>
concept ith_tuple_element_viewer = requires(View &&view) {
  { get<I>(UPD_FWD(view)) } -> std::same_as<tuple_element_t<I, View>>;
};

template<typename Tuple>
concept tuple_view = tuple_like2<Tuple> && UPD_ALL_OF_CONCEPT(ith_tuple_element_viewer, Tuple, tuple_size_v<Tuple>);

template<typename>
struct tuple_view_for; // IWYU pragma: keep

} // namespace upd

namespace upd::tuple_views {

using upd::get;

} // namespace upd::tuple_views

template<typename Tuple>
  requires upd::implementation_of<Tuple, upd::tuple_view_for>
struct upd::tuple_like_for<Tuple> {
  using impl_type = upd::tuple_view_for<Tuple>;

  constexpr static auto size = impl_type::size;

  template<std::size_t I>
  using element_type = decltype(impl_type::template get<I>(std::declval<Tuple>()));

  template<std::size_t I, typename Self>
  [[nodiscard]] constexpr static auto get(Self &&self) noexcept(release) -> decltype(auto) {
    return impl_type::template get<I>(UPD_FWD(self));
  }
};
