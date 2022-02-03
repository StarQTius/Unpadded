//! \file
//! \brief Signed representation portability support

#pragma once

#include "sfinae.hpp"

namespace upd {
namespace detail {

//! \brief Interpret a sequence of bytes as an integer using the provided signed number representation
template<typename T, signed_mode Signed_Mode, sfinae::require<Signed_Mode == signed_mode::BUILTIN> = 0>
T from_signed_mode(unsigned long long value) {
  T retval;
  memcpy(&retval, &value, sizeof(retval));
  return retval;
}
template<typename T, signed_mode Signed_Mode, sfinae::require<Signed_Mode == signed_mode::SIGNED_MAGNITUDE> = 0>
T from_signed_mode(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  constexpr auto magnitude_mask = (~0ull >> 8 * (sizeof(value) - sizeof(T))) ^ sign_mask;
  return value & sign_mask ? T(-T(value & magnitude_mask)) : T(value);
}
template<typename T, signed_mode Signed_Mode, sfinae::require<Signed_Mode == signed_mode::ONE_COMPLEMENT> = 0>
T from_signed_mode(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value & sign_mask ? T(-T(~value)) : T(value);
}
template<typename T, signed_mode Signed_Mode, sfinae::require<Signed_Mode == signed_mode::TWO_COMPLEMENT> = 0>
T from_signed_mode(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value & sign_mask ? T(-T(~value + 1)) : T(value);
}
template<typename T, signed_mode Signed_Mode, sfinae::require<Signed_Mode == signed_mode::OFFSET_BINARY> = 0>
T from_signed_mode(unsigned long long value) {
  constexpr auto offset = 0b10000000ull << 8 * (sizeof(T) - 1);
  return T(value - offset);
}

//! \brief Serialize an integer using the provided signed number representation
template<signed_mode Signed_Mode, typename T, sfinae::require<Signed_Mode == signed_mode::BUILTIN> = 0>
unsigned long long to_signed_mode(T value) {
  unsigned long long retval;
  memcpy(&retval, &value, sizeof value);

  return retval;
}
template<signed_mode Signed_Mode, typename T, sfinae::require<Signed_Mode == signed_mode::SIGNED_MAGNITUDE> = 0>
unsigned long long to_signed_mode(T value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value >= 0 ? static_cast<unsigned long long>(value) : static_cast<unsigned long long>(-value) | sign_mask;
}
template<signed_mode Signed_Mode, typename T, sfinae::require<Signed_Mode == signed_mode::ONE_COMPLEMENT> = 0>
unsigned long long to_signed_mode(T value) {
  return value >= 0 ? static_cast<unsigned long long>(value) : ~static_cast<unsigned long long>(-value);
}
template<signed_mode Signed_Mode, typename T, sfinae::require<Signed_Mode == signed_mode::TWO_COMPLEMENT> = 0>
unsigned long long to_signed_mode(T value) {
  return value >= 0 ? static_cast<unsigned long long>(value) : static_cast<unsigned long long>(~(-value - 1));
}
template<signed_mode Signed_Mode, typename T, sfinae::require<Signed_Mode == signed_mode::OFFSET_BINARY> = 0>
unsigned long long to_signed_mode(T value) {
  constexpr auto offset = 0b10000000ull << 8 * (sizeof(T) - 1);
  return static_cast<unsigned long long>(value + offset);
}

} // namespace detail
} // namespace upd
