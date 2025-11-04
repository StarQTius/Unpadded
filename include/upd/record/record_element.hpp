#pragma once

namespace upd {

template<auto, typename>
struct record_element; // IWYU pragma: keep

template<auto Tag, typename Record>
using record_element_t = typename record_element<Tag, Record>::type;

} // namespace upd

template<auto Tag, typename Record>
struct upd::record_element<Tag, const Record> {
  using type = typename record_element<Tag, Record>::type;
};

template<auto Tag, typename Record>
struct upd::record_element<Tag, Record &> {
  using type = typename record_element<Tag, Record>::type;
};

template<auto Tag, typename Record>
struct upd::record_element<Tag, Record &&> {
  using type = typename record_element<Tag, Record>::type;
};
