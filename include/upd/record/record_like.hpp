#pragma once

#include <concepts>
#include <cstddef>

#include "../upd.hpp"
#include "../variadic_concept.hpp"
#include "record_element.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<typename Record, std::size_t I>
concept ith_tag_gettable =
    requires(Record rec) { record_tag<I, Record>::value; } && requires(Record rec, record_tag<I, Record> tag) {
      typename record_element<tag.value, Record>::type;
    } && requires(Record rec, record_tag<I, Record> tag, record_element<tag.value, Record> elem) {
      { get<tag.value>(UPD_FWD(rec)) } -> std::common_reference_with<typename decltype(elem)::type>;
    };

template<typename Record>
concept record_like = requires(Record rec) {
  record_size<Record>::value;
  { record_size<Record>::value } -> std::equality_comparable_with<std::size_t>;
} && UPD_ALL_OF_CONCEPT(ith_tag_gettable, Record, record_size<Record>::value);

} // namespace upd
