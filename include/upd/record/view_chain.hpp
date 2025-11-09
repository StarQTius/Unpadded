#pragma once

#include <concepts>
#include <tuple>

#include "../constexpr.hpp"
#include "../is_instance_of.hpp"
#include "../upd.hpp"
#include "record_like.hpp"
#include "record_view_adaptor.hpp"

namespace upd {

template<typename... ViewAdaptors>
  requires(is_instance_of<ViewAdaptors, record_view_adaptor_t>() && ...)
struct view_chain {
  template<typename... VA>
    requires(std::convertible_to<VA, ViewAdaptors> && ...)
  explicit constexpr view_chain(VA &&...adaps) : view_adaptors{UPD_FWD(adaps)...} {};

  std::tuple<ViewAdaptors...> view_adaptors;
};

template<typename... ViewAdaptors>
  requires(is_instance_of<ViewAdaptors, record_view_adaptor_t>() && ...)
explicit view_chain(ViewAdaptors...) -> view_chain<ViewAdaptors...>;

template<record_like Record, typename ViewChain>
  requires(is_instance_of<ViewChain, view_chain>())
[[nodiscard]] constexpr auto operator|(Record &&rec, ViewChain &&chain) {
  return std::apply([&](auto &&...adaps) { return (rec | ... | adaps); }, UPD_FWD(chain).view_adaptors);
}

} // namespace upd
