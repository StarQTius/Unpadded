#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>

#include "../named_value.hpp"
#include "../transfert_reference.hpp"
#include "../upd.hpp"
#include "../variadic_concept.hpp"

namespace upd {

template<auto, typename>
struct record_element {};

template<typename>
struct record_size {};

template<std::size_t, typename>
struct record_tag {};

} // namespace upd

namespace upd::detail {

template<typename Record, std::size_t I>
concept ith_tag_gettable = requires(std::remove_reference_t<Record> rec) { record_tag<I, decltype(rec)>::value; } &&
                           requires(std::remove_reference_t<Record> rec, record_tag<I, decltype(rec)> tag) {
                             typename record_element<tag.value, decltype(rec)>::type;
                           } &&
                           requires(std::remove_reference_t<Record> rec,
                                    record_tag<I, decltype(rec)> tag,
                                    record_element<tag.value, decltype(rec)> elem) {
                             {
                               get<tag.value>(UPD_FWD(rec))
                             } -> std::same_as<transfert_reference_t<typename decltype(elem)::type &&, Record &&>>;
                           };

} // namespace upd::detail

namespace upd {

template<typename Record>
concept record_like = requires(std::remove_reference_t<Record> rec) {
  record_size<decltype(rec)>::value;
  { record_size<decltype(rec)>::value } -> std::equality_comparable_with<std::size_t>;
} && UPD_ALL_OF_CONCEPT(detail::ith_tag_gettable, Record, record_size<std::remove_reference_t<Record>>::value);

template<auto Tag, record_like Record>
using record_element_t = typename record_element<Tag, Record>::type;

template<record_like Record>
constexpr auto record_size_v = record_size<Record>::value;

template<std::size_t I, record_like Record>
constexpr auto record_tag_v = record_tag<I, Record>::value;

} // namespace upd
