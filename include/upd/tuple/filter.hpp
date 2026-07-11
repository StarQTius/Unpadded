#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "../utility/type_traits.hpp"
#include "../utility/with_sequence.hpp"
#include "../variadic/clean_occurences_of.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<tuple_like2 Base, typename Pred>
struct filter_view {
  Base base;
  Pred pred;
};

template<tuple_like2 Base, typename Pred>
filter_view(Base &&, Pred) -> filter_view<Base, Pred>;

constexpr auto filter = [](auto &&pred) {
  auto f = [pred = UPD_FWD(pred)]<typename Typebox>(Typebox) {
    constexpr auto keep_it = UPD_INVOKE(pred, Typebox{});
    return expr<keep_it>;
  };

  return tuple_view_adaptor<filter_view>(std::move(f));
};

} // namespace upd::tuple_views

template<upd::tuple_like2 Base, typename Pred>
struct upd::tuple_view_for<upd::tuple_views::filter_view<Base, Pred>> {
  using base_type = Base;

  constexpr static auto indices_to_keep =
      UPD_WITH_SEQUENCE(Is, tuple_size_v<Base>) {
    return clean_occurences_of_v<
        expr_t<false>,
        std::invoke_result_t<Pred, typebox<tuple_element_t<Is, Base> &&>>...>;
  };

  constexpr static auto size = indices_to_keep.size();

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    constexpr auto i = indices_to_keep[I];
    return upd::get<i>(UPD_FWD(view).base);
  }
};
