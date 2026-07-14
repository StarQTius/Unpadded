#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/with_sequence.hpp"
#include "../variadic/clean_occurences_of.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "ith_record_element.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<record_like Base, typename Pred>
struct filter_view {
  Base base;
  Pred pred;
};

template<record_like Base, typename Pred>
filter_view(Base &&, Pred) -> filter_view<Base, Pred>;

constexpr auto filter = [](auto &&pred) {
  auto p = [pred = UPD_FWD(pred)]<auto Tag, typename T> {
    constexpr auto keep_it = pred.template operator()<Tag, T>();
    return expr<keep_it>;
  };

  return record_view_adaptor<filter_view>(UPD_FWD(p));
};

} // namespace upd::record_views

template<upd::record_like Base, typename Pred>
struct upd::record_view_for<upd::record_views::filter_view<Base, Pred>> {
  using base_type = Base;

  template<std::size_t I>
  constexpr static auto ith_tag_v = record_tag_v<I, Base>;

  template<std::size_t I>
  using ith_arg_t = std::remove_cvref_t<ith_record_element_t<I, Base>>;

  constexpr static auto indices_to_keep =
      UPD_WITH_SEQUENCE(Is, record_size_v<Base>) {
    return clean_occurences_of_v<
        expr_t<false>, decltype(std::declval<Pred>().template
                                operator()<ith_tag_v<Is>, ith_arg_t<Is>>())...>;
  };

  constexpr static auto size = indices_to_keep.size();

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) -> decltype(auto) {
    constexpr auto i = indices_to_keep[I];
    constexpr auto tag = record_tag_v<i, Base>;
    using type = ith_record_element_t<i, Base>;

    return entry<tag, type>{expr<tag>, upd::get_ith<i>(UPD_FWD(view).base)};
  }
};
