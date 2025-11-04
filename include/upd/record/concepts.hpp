#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>

#include "../named_value.hpp"
#include "../transfert_reference.hpp"
#include "../upd.hpp"
#include "../variadic_concept.hpp"
#include "record_element.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd::detail {

template<typename Record, std::size_t I>
concept ith_tag_gettable =
    requires(Record rec) { record_tag<I, Record>::value; } && requires(Record rec, record_tag<I, Record> tag) {
      typename record_element<tag.value, Record>::type;
    } && requires(Record rec, record_tag<I, Record> tag, record_element<tag.value, Record> elem) {
      { get<tag.value>(UPD_FWD(rec)) } -> std::common_reference_with<typename decltype(elem)::type>;
    };

template<typename View, std::size_t I>
concept ith_element_owned = requires(View view, record_tag<I, View> tag, record_element<tag.value, View> elem) {
  { get<tag.value>(UPD_FWD(view)) } -> std::same_as<transfert_reference_t<typename decltype(elem)::type &&, View &&>>;
};

template<typename View, std::size_t I>
concept ith_element_viewed = requires(View view, record_tag<I, View> tag, record_element<tag.value, View> elem) {
  { get<tag.value>(UPD_FWD(view)) } -> std::same_as<typename decltype(elem)::type>;
};

} // namespace upd::detail

namespace upd {

template<typename Record>
concept record_like = requires(Record rec) {
  record_size<Record>::value;
  { record_size<Record>::value } -> std::equality_comparable_with<std::size_t>;
} && UPD_ALL_OF_CONCEPT(detail::ith_tag_gettable, Record, record_size<Record>::value);

template<typename Record>
concept regular_record =
    record_like<Record> && UPD_ALL_OF_CONCEPT(detail::ith_element_owned, Record, record_size<Record>::value);

template<typename Record>
concept record_view =
    record_like<Record> && UPD_ALL_OF_CONCEPT(detail::ith_element_viewed, Record, record_size<Record>::value);

} // namespace upd

namespace upd::detail {

template<typename Record, std::size_t I>
concept ith_element_record_like = record_like<record_element_t<record_tag_v<I, Record>, Record>>;

} // namespace upd::detail

namespace upd {

template<typename Record>
concept nested_record =
    record_like<Record> &&
    UPD_ALL_OF_CONCEPT(detail::ith_element_record_like, Record, record_size_v<std::remove_reference_t<Record>>);

} // namespace upd
