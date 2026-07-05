#pragma once

#include <cstddef>
#include <tuple>
#include <variant>

#include "../tuple/tuple_element.hpp"
#include "../upd.hpp"
#include "../utility/get.hpp"
#include "../variadic/nested_indices.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
#include "view.hpp"

namespace upd::record_views {

template<record_like... Bases>
struct concat_view {
  explicit constexpr concat_view(Bases &&...bs) : bases{UPD_FWD(bs)...} {}

  std::tuple<Bases...> bases;
};

template<record_like... Bases>
concat_view(Bases &&...) -> concat_view<Bases...>;

constexpr auto concat = []<record_like... Bases> [[nodiscard]] (Bases &&...bases) noexcept(release) {
  auto ensure_view = [](auto &&rec) { return view_t{UPD_FWD(rec)}; };
  return concat_view{ensure_view(UPD_FWD(bases))...};
};

} // namespace upd::record_views

template<upd::record_like... Bases>
struct upd::record_view_for<upd::record_views::concat_view<Bases...>> {
  constexpr static auto size = (record_size_v<Bases> + ... + 0zu);

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) {
    constexpr auto ni = nested_indices_v<record_size_v<Bases>...>[I];
    constexpr auto i = ni.second;
    constexpr auto j = ni.first;

    decltype(auto) value = upd::get_ith<j>(get<i>(UPD_FWD(view).bases));
    constexpr auto identifier = record_tag_v<j, tuple_element_t<i, std::tuple<Bases...>>>;
    return entry<identifier, decltype(value)>{UPD_FWD(value)};
  }
};
