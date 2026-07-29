#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "../detail/fail_unless_discarded.hpp"
#include "../upd.hpp"
#include "../utility/type_traits.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "ith_record_element.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "record_view.hpp"
#include "record_view_adaptor.hpp"

namespace upd::record_views {

template<record_like Base, typename F>
struct transform_view {
  Base base;
  F f;
};

template<record_like Base, typename F>
transform_view(Base &&, F) -> transform_view<Base, F>;

constexpr auto transform = [](auto &&f) {
  auto fn = [f = UPD_FWD(f)]<auto Tag, typename T>(T &&v) -> decltype(auto) {
    return UPD_INVOKE_TEMPLATE(f, (Tag), UPD_FWD(v));
  };

  return record_view_adaptor<transform_view>(UPD_FWD(fn));
};

template<record_like Base, typename F>
struct transform_type_view {
  Base base;
  F f;
};

template<record_like Base, typename F>
transform_type_view(Base &&, F) -> transform_type_view<Base, F>;

constexpr auto transform_type = record_view_adaptor<transform_type_view>;

} // namespace upd::record_views

template<upd::record_like Base, typename F>
struct upd::record_view_for<upd::record_views::transform_view<Base, F>> {
  using base_type = Base;

  constexpr static auto size = record_size_v<Base>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(View &&view) -> decltype(auto) {
    constexpr auto tag = record_tag_v<I, Base>;
    using arg_t = ith_record_element_t<I, Base>;
    using type = decltype(UPD_INVOKE_TEMPLATE(std::declval<F>(), (tag),
                                              std::declval<arg_t>()));
    return entry<tag, type>{UPD_INVOKE_TEMPLATE(
        view.f, (tag), upd::get_ith<I>(UPD_FWD(view).base))};
  }
};

template<upd::record_like Base, typename F>
struct upd::record_view_for<upd::record_views::transform_type_view<Base, F>> {
  using base_type = Base;

  constexpr static auto size = record_size_v<Base>;

  template<std::size_t I>
  using ith_arg_t = std::remove_reference_t<ith_record_element_t<I, Base>>;

  template<std::size_t I>
  using ith_result_t = decltype(UPD_INVOKE_TEMPLATE(
      std::declval<F>(), (record_tag_v<I, Base>, ith_arg_t<I>)));

  template<std::size_t I>
  using ith_entry_t = entry<record_tag_v<I, Base>, ith_result_t<I>>;

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(const View &) -> ith_entry_t<I> {
    detail::fail_unless_discarded(
        "This function cannot be called in evaluated context");
  }

  template<std::size_t I, typename View>
  [[nodiscard]] constexpr static auto get_ith(const View &)
    requires metavalue<ith_result_t<I>>
  {
    return ith_entry_t<I>{};
  }
};
