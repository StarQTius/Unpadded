#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../get.hpp"
#include "../upd.hpp"
#include "../variadic/nested_indices.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "view.hpp"

namespace upd::tuple_views {

template<tuple_like2... Bases>
struct concat_view {
  explicit constexpr concat_view(Bases &&...bs) : bases{UPD_FWD(bs)...} {}

  std::tuple<Bases...> bases;
};

template<tuple_like2... Bases>
concat_view(Bases &&...) -> concat_view<Bases...>;

constexpr auto concat = []<tuple_like2... Bases> [[nodiscard]] (Bases &&...bases) noexcept(release) {
  auto ensure_view = [](auto &&rec) { return view_t{UPD_FWD(rec)}; };
  return concat_view{ensure_view(UPD_FWD(bases))...};
};

} // namespace upd::tuple_views

template<upd::tuple_like2... Bases>
struct upd::tuple_view_for<upd::tuple_views::concat_view<Bases...>> {
  constexpr static auto size = (tuple_size_v<Bases> + ... + 0zu);

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    constexpr auto ni = nested_indices_v<tuple_size_v<Bases>...>[I];
    constexpr auto i = ni.second;
    constexpr auto j = ni.first;

    using intermediate_type = decltype(upd::get<i>(UPD_FWD(view).bases));

    if constexpr (std::is_reference_v<intermediate_type>) {
      return upd::get<j>(upd::get<i>(UPD_FWD(view).bases));
    } else {
      return auto{upd::get<j>(upd::get<i>(UPD_FWD(view).bases))};
    }
  }
};
