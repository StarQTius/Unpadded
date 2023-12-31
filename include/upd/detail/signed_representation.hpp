//! \file

#pragma once

#include "../format.hpp"
#include "../upd.hpp"
#include "type_traits/require.hpp"

// NOLINTBEGIN

namespace upd {
namespace detail {

//! \brief Platform-agnostic implementations of `from_signed_mode`
template<typename T, signed_mode Signed_Mode, detail::require<Signed_Mode == signed_mode::SIGNED_MAGNITUDE> = 0>
T from_signed_mode_impl(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  constexpr auto magnitude_mask = (~0ull >> 8 * (sizeof(value) - sizeof(T))) ^ sign_mask;
  return value & sign_mask ? T(-T(value & magnitude_mask)) : T(value);
}
template<typename T, signed_mode Signed_Mode, detail::require<Signed_Mode == signed_mode::ONES_COMPLEMENT> = 0>
T from_signed_mode_impl(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value & sign_mask ? T(-T(~value)) : T(value);
}
template<typename T, signed_mode Signed_Mode, detail::require<Signed_Mode == signed_mode::TWOS_COMPLEMENT> = 0>
T from_signed_mode_impl(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value & sign_mask ? T(-T(~value + 1)) : T(value);
}
template<typename T, signed_mode Signed_Mode, detail::require<Signed_Mode == signed_mode::OFFSET_BINARY> = 0>
T from_signed_mode_impl(unsigned long long value) {
  constexpr auto offset = 0b10000000ull << 8 * (sizeof(T) - 1);
  return T(value - offset);
}

//! \brief Interpret a sequence of bytes as an integer using the provided signed number representation
template<typename T, signed_mode Signed_Mode, UPD_REQUIRE(platform_info.signed_mode == Signed_Mode)>
T from_signed_mode(unsigned long long value) {
  T retval;
  memcpy(&retval, &value, sizeof(retval));
  return retval;
}
template<typename T, signed_mode Signed_Mode, UPD_REQUIRE(platform_info.signed_mode != Signed_Mode)>
T from_signed_mode(unsigned long long value) {
  return from_signed_mode_impl<T, Signed_Mode>(value);
}

//! \brief Platform-agnostic implementations of `to_signed_mode`
template<signed_mode Signed_Mode, typename T, detail::require<Signed_Mode == signed_mode::SIGNED_MAGNITUDE> = 0>
unsigned long long to_signed_mode_impl(T value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value >= 0 ? static_cast<unsigned long long>(value) : static_cast<unsigned long long>(-value) | sign_mask;
}
template<signed_mode Signed_Mode, typename T, detail::require<Signed_Mode == signed_mode::ONES_COMPLEMENT> = 0>
unsigned long long to_signed_mode_impl(T value) {
  return value >= 0 ? static_cast<unsigned long long>(value) : ~static_cast<unsigned long long>(-value);
}
template<signed_mode Signed_Mode, typename T, detail::require<Signed_Mode == signed_mode::TWOS_COMPLEMENT> = 0>
unsigned long long to_signed_mode_impl(T value) {
  return value >= 0 ? static_cast<unsigned long long>(value) : static_cast<unsigned long long>(~(-value - 1));
}
template<signed_mode Signed_Mode, typename T, detail::require<Signed_Mode == signed_mode::OFFSET_BINARY> = 0>
unsigned long long to_signed_mode_impl(T value) {
  constexpr auto offset = 0b10000000ull << 8 * (sizeof(T) - 1);
  return static_cast<unsigned long long>(value + offset);
}

//! \brief Serialize an integer using the provided signed number representation
template<signed_mode Signed_Mode, typename T, UPD_REQUIRE(platform_info.signed_mode == Signed_Mode)>
unsigned long long to_signed_mode(T value) {
  unsigned long long retval;
  memcpy(&retval, &value, sizeof value);

  return retval;
}
template<signed_mode Signed_Mode, typename T, UPD_REQUIRE(platform_info.signed_mode != Signed_Mode)>
unsigned long long to_signed_mode(T value) {
  return to_signed_mode_impl<Signed_Mode, T>(value);
}

} // namespace detail
} // namespace upd

// NOLINTEND
