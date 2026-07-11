#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <utility>

#include "../upd.hpp"
#include "../utility/get.hpp"
#include "../utility/type_traits.hpp"
#include "../utility/with_sequence.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"
#include "tuple_view.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<tuple_like2 Base>
struct unique_view {
  Base base;
};

template<tuple_like2 Base>
unique_view(Base &&) -> unique_view<Base>;

constexpr auto unique = tuple_view_adaptor<unique_view>();

} // namespace upd::tuple_views

template<upd::tuple_like2 Base>
struct upd::tuple_view_for<upd::tuple_views::unique_view<Base>> {
  using base_type = Base;

  constexpr static auto addrs_and_size =
      UPD_WITH_SEQUENCE(Is, tuple_size_v<Base>) {
    namespace stdr = std::ranges;

    auto retval =
        std::array{std::pair{uid<std::tuple_element_t<Is, Base>>, Is}...};
    stdr::sort(retval);

    auto remainder = stdr::unique(retval, std::equal_to{}, upd::get<0>);
    stdr::sort(retval.begin(), remainder.begin(), std::less{}, upd::get<1>);

    return std::pair{retval, retval.size() - remainder.size()};
  };

  constexpr static auto size = addrs_and_size.second;

  constexpr static auto indices = UPD_WITH_SEQUENCE(Is, size) {
    return std::array{addrs_and_size.first[Is].second...};
  };

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get(View &&view) -> decltype(auto) {
    return upd::get<indices[I]>(UPD_FWD(view).base);
  }
};
