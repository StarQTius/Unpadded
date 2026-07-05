#pragma once

#include <concepts>
#include <tuple>

#include "../upd.hpp"
#include "../utility/is_instance_of.hpp"
#include "tuple_like.hpp"
#include "tuple_view_adaptor.hpp"

namespace upd::tuple_views {

template<typename... ViewAdaptors>
  requires(is_instance_of<ViewAdaptors, tuple_view_adaptor_t>() && ...)
struct chain {
  template<typename... VA>
    requires(std::convertible_to<VA, ViewAdaptors> && ...)
  explicit constexpr chain(VA &&...adaps) : view_adaptors{UPD_FWD(adaps)...} {};

  std::tuple<ViewAdaptors...> view_adaptors;
};

template<typename... ViewAdaptors>
  requires(is_instance_of<ViewAdaptors, tuple_view_adaptor_t>() && ...)
explicit chain(ViewAdaptors...) -> chain<ViewAdaptors...>;

template<tuple_like2 Tuple, typename ViewChain>
  requires(is_instance_of<ViewChain, chain>())
[[nodiscard]] constexpr auto operator|(Tuple &&t, ViewChain &&chain) {
  return std::apply([&](auto &&...adaps) { return (UPD_FWD(t) | ... | UPD_FWD(adaps)); }, UPD_FWD(chain).view_adaptors);
}

} // namespace upd::tuple_views
