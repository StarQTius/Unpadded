#pragma once

#include <cstddef>

#include "../get.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "../variadic/clean_occurences_of.hpp"
#include "../with_sequence.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<tuple_like2 Base, typename T>
struct clean_view {
  Base base;

  explicit constexpr clean_view(Base b, typebox<T>) : base{UPD_FWD(b)} {}
};

template<tuple_like2 Base, typename T>
clean_view(Base &&, typebox<T>) -> clean_view<Base, T>;

template<typename T, auto Typebox = typebox<T>{}>
constexpr auto clean = tuple_view_adaptor<clean_view>(Typebox);

} // namespace upd::tuple_views

template<upd::tuple_like2 Base, typename T>
struct upd::tuple_view_for<upd::tuple_views::clean_view<Base, T>> {
  using base_type = Base;

  constexpr static auto indices_to_keep = UPD_WITH_SEQUENCE(Is, tuple_size_v<Base>) {
    return clean_occurences_of_v<T, tuple_element_t<Is, Base>...>;
  };

  constexpr static auto size = indices_to_keep.size();

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    constexpr auto i = indices_to_keep[I];
    return upd::get<i>(UPD_FWD(view).base);
  }
};
