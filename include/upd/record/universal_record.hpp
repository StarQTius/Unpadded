#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include "../upd.hpp"
#include "record_like.hpp"

namespace upd {

template<typename T>
struct universal_record {
  explicit constexpr universal_record(const T &v) : value{v} {}
  explicit constexpr universal_record(T &&v) : value{std::move(v)} {}

  T value;
};

template<auto Tag, typename T>
[[nodiscard]] constexpr auto has_tag(const universal_record<T> &) noexcept(release) -> bool {
  return true;
}

template<typename U, typename T>
[[nodiscard]] constexpr auto has_type(const universal_record<T> &) noexcept(release) -> bool {
  return std::same_as<T, U>;
}

} // namespace upd

template<typename T>
struct upd::record_like_for<upd::universal_record<T>> {
  constexpr static auto size = 0zu;

  template<std::size_t I, typename Record>
  [[nodiscard]] constexpr static auto get_ith(const Record &rec) noexcept(release) -> const auto & {
    return UPD_FWD(rec).value;
  }
};
