#pragma once

namespace upd {

template<typename>
struct record_size; // IWYU pragma: keep

template<typename Record>
constexpr auto record_size_v = record_size<Record>::value;

} // namespace upd

template<typename Record>
struct upd::record_size<const Record> {
  constexpr static auto value = record_size<Record>::value;
};

template<typename Record>
struct upd::record_size<Record &> {
  constexpr static auto value = record_size<Record>::value;
};

template<typename Record>
struct upd::record_size<Record &&> {
  constexpr static auto value = record_size<Record>::value;
};
