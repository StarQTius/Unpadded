#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

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
                           requires(Record rec,
                                    std::remove_reference_t<Record> rec_,
                                    record_tag<I, decltype(rec_)> tag,
                                    record_element<tag.value, decltype(rec_)> elem) {
                             {
                               get<tag.value>(UPD_FWD(rec))
                             } -> std::convertible_to<const typename decltype(elem)::type &>;
                           };

template<typename View, std::size_t I>
concept ith_element_owned = requires(View view,
                                     std::remove_reference_t<View> view_,
                                     record_tag<I, decltype(view_)> tag,
                                     record_element<tag.value, decltype(view_)> elem) {
  { get<tag.value>(UPD_FWD(view)) } -> std::same_as<transfert_reference_t<typename decltype(elem)::type &&, View &&>>;
};

template<typename View, std::size_t I>
concept ith_element_viewed = requires(View view,
                                      std::remove_reference_t<View> view_,
                                      record_tag<I, decltype(view_)> tag,
                                      record_element<tag.value, decltype(view_)> elem) {
  { get<tag.value>(UPD_FWD(view)) } -> std::same_as<typename decltype(elem)::type>;
};

} // namespace upd::detail

namespace upd {

template<typename Record>
concept record_like = requires(std::remove_reference_t<Record> rec) {
  record_size<decltype(rec)>::value;
  { record_size<decltype(rec)>::value } -> std::equality_comparable_with<std::size_t>;
} && UPD_ALL_OF_CONCEPT(detail::ith_tag_gettable, Record, record_size<std::remove_reference_t<Record>>::value);

template<typename Record>
concept regular_record =
    record_like<Record> &&
    UPD_ALL_OF_CONCEPT(detail::ith_element_owned, Record, record_size<std::remove_reference_t<Record>>::value);

template<typename Record>
concept record_view =
    record_like<Record> &&
    UPD_ALL_OF_CONCEPT(detail::ith_element_viewed, Record, record_size<std::remove_reference_t<Record>>::value);

template<auto Tag, record_like Record>
using record_element_t = typename record_element<Tag, Record>::type;

template<record_like Record>
constexpr auto record_size_v = record_size<Record>::value;

template<std::size_t I, record_like Record>
constexpr auto record_tag_v = record_tag<I, Record>::value;

template<typename>
struct record_view_for; // IWYU pragma: keep

template<typename T, template<typename> typename Traits>
concept implementation_of = requires {
  typename Traits<std::remove_cvref_t<T>>;
  Traits<std::remove_cvref_t<T>>{};
};

} // namespace upd

template<auto Id, typename View>
  requires upd::implementation_of<View, upd::record_view_for>
struct upd::record_element<Id, View> {
  using type = decltype(get<Id>(std::declval<View>()));
};

template<std::size_t I, typename View>
  requires upd::implementation_of<View, upd::record_view_for>
struct upd::record_tag<I, View> {
  constexpr static auto value =
      upd::record_tag_v<I, std::remove_cvref_t<typename upd::record_view_for<std::remove_cvref_t<View>>::base_type>>;
};

template<typename View>
  requires upd::implementation_of<View, upd::record_view_for>
struct upd::record_size<View> {
  constexpr static auto value =
      upd::record_size_v<std::remove_cvref_t<typename upd::record_view_for<std::remove_cvref_t<View>>::base_type>>;
};

template<auto Id, typename View>
  requires upd::implementation_of<View, upd::record_view_for>
[[nodiscard]] constexpr auto get(View &&view) -> decltype(auto) {
  return upd::record_view_for<std::remove_cvref_t<View>>::template get<Id>(UPD_FWD(view));
}
