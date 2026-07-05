#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>

#include "../upd.hpp"
#include "../utility/get.hpp"
#include "../utility/implementation_of.hpp"
#include "../utility/variadic_concept.hpp"
#include "record_element.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<typename Record, std::size_t I>
concept ith_record_tag_gettable =
    requires(Record rec) { record_tag<I, Record>::value; } && requires(Record rec, record_tag<I, Record> tag) {
      typename record_element<tag.value, Record>::type;
    } && requires(Record rec, record_tag<I, Record> tag, record_element<tag.value, Record> elem) {
      { get<tag.value>(UPD_FWD(rec)) } -> std::common_reference_with<typename decltype(elem)::type &&>;
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

} // namespace upd
