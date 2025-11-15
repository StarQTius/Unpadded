#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>

#include "../constexpr.hpp"
#include "../implementation_of.hpp"
#include "../upd.hpp"
#include "../variadic_concept.hpp"
#include "../with_sequence.hpp"
#include "lite_record.hpp"
#include "record_element.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<typename Record, std::size_t I>
concept ith_record_tag_gettable =
    requires(Record rec) { record_tag<I, Record>::value; } && requires(Record rec, record_tag<I, Record> tag) {
      typename record_element<tag.value, Record>::type;
    } && requires(Record rec, record_tag<I, Record> tag, record_element<tag.value, Record> elem) {
      { get<tag.value>(UPD_FWD(rec)) } -> std::common_reference_with<typename decltype(elem)::type>;
    };

template<typename Record>
concept record_like = requires(Record rec) {
  record_size<std::remove_cvref_t<Record>>::value;
  { record_size<Record>::value } -> std::equality_comparable_with<std::size_t>;
} && UPD_ALL_OF_CONCEPT(ith_record_tag_gettable, Record, record_size<Record>::value);

template<typename>
struct record_like_for; // IWYU pragma: keep

template<typename Record>
  requires implementation_of<Record, record_like_for>
struct record_size<Record> {
  constexpr static auto value = record_like_for<std::remove_cvref_t<Record>>::size;
};

template<std::size_t I, typename Record>
  requires implementation_of<Record, record_like_for>
struct record_tag<I, Record> {
  constexpr static auto value = record_like_for<std::remove_cvref_t<Record>>::template tag<I>;
};

template<auto Tag, typename Record>
  requires implementation_of<Record, record_like_for>
struct record_element<Tag, Record> {
  using type = typename record_like_for<std::remove_cvref_t<Record>>::template element_type<Tag>;
};

template<auto Tag, typename Record>
  requires implementation_of<Record, record_like_for>
[[nodiscard]] constexpr auto get(Record &&rec) -> decltype(auto) {
  using record_type = std::remove_cvref_t<Record>;
  using impl_type = upd::record_like_for<record_type>;

  constexpr auto i = UPD_WITH_SEQUENCE(Is, record_size<Record>::value) {
    auto lut = lite_record{lite_record_node{expr<upd::record_tag<Is, Record>::value>, Is}...};
    return get<Tag>(lut);
  };

  decltype(auto) retval = impl_type::template get_ith<i>(UPD_FWD(rec));
  using retval_type = decltype(retval);

  if constexpr (std::is_reference_v<retval_type>) {
    return UPD_FWD(retval);
  } else {
    return retval;
  }
}

} // namespace upd
