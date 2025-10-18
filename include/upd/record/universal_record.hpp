#pragma once

#include <concepts>
#include <cstddef>
#include <utility>

#include "../upd.hpp"
#include "concepts.hpp"

namespace upd {

template<typename T>
struct universal_record {
  explicit constexpr universal_record(const T &v) : value{v} {}
  explicit constexpr universal_record(T &&v) : value{std::move(v)} {}

  T value;
};

template<auto Tag, typename T>
struct record_element<Tag, universal_record<T>> {
  using type = T;
};

template<std::size_t I, typename T>
struct record_tag<I, universal_record<T>> {};

template<typename T>
struct record_size<universal_record<T>> {
  constexpr static auto value = 0zu;
};

template<auto Tag, typename T>
[[nodiscard]] constexpr auto get(const universal_record<T> &rec) noexcept(release) -> const T & {
  return rec.value;
}

template<auto Tag, typename T>
[[nodiscard]] constexpr auto get(const universal_record<T> &&rec) noexcept(release) -> const T && {
  return std::move(rec).value;
}

template<auto Tag, typename T>
[[nodiscard]] constexpr auto has_tag(const universal_record<T> &) noexcept(release) -> bool {
  return true;
}

template<typename U, typename T>
[[nodiscard]] constexpr auto has_type(const universal_record<T> &) noexcept(release) -> bool {
  return std::same_as<T, U>;
}

} // namespace upd
