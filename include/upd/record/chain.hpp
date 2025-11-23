#pragma once

#include <concepts>
#include <tuple>

#include "../is_instance_of.hpp"
#include "../upd.hpp"
#include "record_like.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<typename... ViewAdaptors>
  requires(is_instance_of<ViewAdaptors, record_view_adaptor_t>() && ...)
struct chain {
  template<typename... VA>
    requires(std::convertible_to<VA, ViewAdaptors> && ...)
  explicit constexpr chain(VA &&...adaps) : view_adaptors{UPD_FWD(adaps)...} {};

  std::tuple<ViewAdaptors...> view_adaptors;
};

template<typename... ViewAdaptors>
  requires(is_instance_of<ViewAdaptors, record_view_adaptor_t>() && ...)
explicit chain(ViewAdaptors...) -> chain<ViewAdaptors...>;

template<record_like Record, typename ViewChain>
  requires(is_instance_of<ViewChain, chain>())
[[nodiscard]] constexpr auto operator|(Record &&rec, ViewChain &&chain) {
  return std::apply([&](auto &&...adaps) { return (UPD_FWD(rec) | ... | UPD_FWD(adaps)); },
                    UPD_FWD(chain).view_adaptors);
}

} // namespace upd::record_views
