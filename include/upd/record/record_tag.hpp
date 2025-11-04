#pragma once

#include <cstddef>

namespace upd {

template<std::size_t, typename>
struct record_tag; // IWYU pragma: keep

template<std::size_t I, typename Record>
constexpr auto record_tag_v = record_tag<I, Record>::value;

} // namespace upd

template<std::size_t I, typename Record>
struct upd::record_tag<I, const Record> {
  constexpr static auto value = record_tag<I, Record>::value;
};

template<std::size_t I, typename Record>
struct upd::record_tag<I, Record &> {
  constexpr static auto value = record_tag<I, Record>::value;
};

template<std::size_t I, typename Record>
struct upd::record_tag<I, Record &&> {
  constexpr static auto value = record_tag<I, Record>::value;
};
