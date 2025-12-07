#pragma once

#include <cstddef>

#include "../type_traits.hpp"
#include "../upd.hpp"
#include "../variadic/clean_occurences_of.hpp"
#include "../with_sequence.hpp"
#include "entry.hpp"
#include "ith_record_element.hpp"
#include "lite_record.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<record_like Base, typename T>
struct clean_view {
  Base base;

  explicit constexpr clean_view(Base b, typebox<T>) : base{UPD_FWD(b)} {}
};

template<record_like Base, typename T>
clean_view(Base &&, typebox<T>) -> clean_view<Base, T>;

template<typename T, auto Typebox = typebox<T>{}>
constexpr auto clean = record_view_adaptor<clean_view>(Typebox);

} // namespace upd::record_views

template<upd::record_like Base, typename T>
struct upd::record_view_for<upd::record_views::clean_view<Base, T>> {
  using base_type = Base;

  constexpr static auto indices_to_keep = UPD_WITH_SEQUENCE(Is, record_size_v<Base>) {
    return clean_occurences_of_v<T, ith_record_element_t<Is, Base>...>;
  };

  constexpr static auto size = indices_to_keep.size();

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) -> decltype(auto) {
    constexpr auto tag = record_tag_v<indices_to_keep[I], Base>;
    using type = decltype(get<tag>(UPD_FWD(view).base));
    return entry<tag, type>{get<tag>(UPD_FWD(view).base)};
  }
};
