#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "../detail/fail_unless_discarded.hpp"
#include "../functional.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<tuple_like2 Base, typename F>
struct transform_view {
  Base base;
  F f;
};

template<tuple_like2 Base, typename F>
transform_view(Base &&, F) -> transform_view<Base, F>;

constexpr auto transform = tuple_view_adaptor<transform_view>;

template<tuple_like2 Base, typename F>
struct transform_type_view {
  Base base;
  F f;
};

template<tuple_like2 Base, typename F>
transform_type_view(Base &&, F) -> transform_type_view<Base, F>;

constexpr auto transform_type = tuple_view_adaptor<transform_type_view>;

} // namespace upd::tuple_views

template<upd::tuple_like2 Base, typename F>
struct upd::tuple_view_for<upd::tuple_views::transform_view<Base, F>> {
  using base_type = Base;

  constexpr static auto size = upd::tuple_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return UPD_INVOKE(view.f, upd::get<I>(UPD_FWD(view).base));
  }
};

template<upd::tuple_like2 Base, typename F>
struct upd::tuple_view_for<upd::tuple_views::transform_type_view<Base, F>> {
  using base_type = Base;

  template<std::size_t I>
  using ith_arg_t = std::remove_reference_t<tuple_element_t<I, Base>>;

  template<std::size_t I>
  using ith_result_t = decltype(std::declval<F>().template operator()<ith_arg_t<I>>());

  constexpr static auto size = upd::tuple_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(const View &) -> ith_result_t<I> {
    detail::fail_unless_discarded("This function cannot be called in evaluated context");
  }

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(const View &)
    requires metavalue<ith_result_t<I>>
  {
    return ith_result_t<I>{};
  }
};
