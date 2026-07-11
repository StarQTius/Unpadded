#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "../utility/implementation_of.hpp"
#include "../utility/variadic_concept.hpp"
#include "../utility/with_sequence.hpp"
#include "lite_record.hpp"
#include "record_element.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<typename View, std::size_t I>
concept ith_record_element_viewer = requires(
    View view, record_tag<I, View> tag, record_element<tag.value, View> elem) {
  {
    get<tag.value>(UPD_FWD(view))
  } -> std::same_as<typename decltype(elem)::type>;
};

template<typename Record>
concept record_view =
    record_like<Record>
    && UPD_ALL_OF_CONCEPT(
        ith_record_element_viewer, Record, record_size<Record>::value);

template<typename>
struct record_view_for; // IWYU pragma: keep

} // namespace upd

template<typename Record>
  requires upd::implementation_of<Record, upd::record_view_for>
struct upd::record_like_for<Record> {
  using impl_type = record_view_for<Record>;

  constexpr static auto size = impl_type::size;

  template<std::size_t I>
  using ith_entry_type =
      decltype(impl_type::template get_ith<I>(std::declval<Record>()));

  template<std::size_t I>
  constexpr static auto tag = ith_entry_type<I>::identifier;

  template<auto Tag>
  constexpr static auto tag_index = UPD_WITH_SEQUENCE(Is, size) {
    auto lut = lite_record{lite_record_node{expr<tag<Is>>, Is}...};
    return get<Tag>(lut);
  };

  template<auto Tag>
  using element_type = typename ith_entry_type<tag_index<Tag>>::value_type;

  template<std::size_t I, typename Self>
  [[nodiscard]] constexpr static auto
  get_ith(Self &&self) noexcept(release) -> decltype(auto) {
    using retval_type =
        decltype(impl_type::template get_ith<I>(UPD_FWD(self)).value);

    if constexpr (std::is_reference_v<retval_type>) {
      return UPD_FWD(impl_type::template get_ith<I>(UPD_FWD(self)).value);
    } else {
      return impl_type::template get_ith<I>(UPD_FWD(self)).value;
    }
  }
};
