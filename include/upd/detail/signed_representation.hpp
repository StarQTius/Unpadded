//! \file
//! \brief Signed representation portability support

#pragma once

#include "sfinae.hpp"

namespace upd {
namespace detail {

//! \brief Interpret a sequence of bytes as an integer using the provided signed number representation
template<typename T, signed_mode Signed_Mode>
sfinae::enable_t<Signed_Mode == signed_mode::BUILTIN, T> interpret_from(unsigned long long value) {
  T retval;
  memcpy(&retval, &value, sizeof(retval));

  return retval;
}
template<typename T, signed_mode Signed_Mode>
sfinae::enable_t<Signed_Mode == signed_mode::SIGNED_MAGNITUDE, T> interpret_from(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  constexpr auto magnitude_mask = (~0ull >> 8 * (sizeof(value) - sizeof(T))) ^ sign_mask;
  return value & sign_mask ? -static_cast<T>(value & magnitude_mask) : static_cast<T>(value);
}
template<typename T, signed_mode Signed_Mode>
sfinae::enable_t<Signed_Mode == signed_mode::ONE_COMPLEMENT, T> interpret_from(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value & sign_mask ? -static_cast<T>(~value) : static_cast<T>(value);
}
template<typename T, signed_mode Signed_Mode>
sfinae::enable_t<Signed_Mode == signed_mode::TWO_COMPLEMENT, T> interpret_from(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value & sign_mask ? -static_cast<T>(~value + 1) : static_cast<T>(value);
}

template<typename T, signed_mode Signed_Mode>
sfinae::enable_t<Signed_Mode == signed_mode::OFFSET_BINARY, T> interpret_from(unsigned long long value) {
  constexpr auto offset = 0b10000000ull << 8 * (sizeof(T) - 1);
  return static_cast<T>(value - offset);
}
//! \brief Serialize an integer using the provided signed number representation
template<signed_mode Signed_Mode, typename T>
sfinae::enable_t<Signed_Mode == signed_mode::BUILTIN, unsigned long long> interpret_to(T value) {
  unsigned long long retval;
  memcpy(&retval, &value, sizeof value);

  return retval;
}
template<signed_mode Signed_Mode, typename T>
sfinae::enable_t<Signed_Mode == signed_mode::SIGNED_MAGNITUDE, unsigned long long> interpret_to(T value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value >= 0 ? static_cast<unsigned long long>(value) : static_cast<unsigned long long>(-value) | sign_mask;
}
template<signed_mode Signed_Mode, typename T>
sfinae::enable_t<Signed_Mode == signed_mode::ONE_COMPLEMENT, unsigned long long> interpret_to(T value) {
  return value >= 0 ? static_cast<unsigned long long>(value) : ~static_cast<unsigned long long>(-value);
}
template<signed_mode Signed_Mode, typename T>
sfinae::enable_t<Signed_Mode == signed_mode::TWO_COMPLEMENT, unsigned long long> interpret_to(T value) {
  return value >= 0 ? static_cast<unsigned long long>(value) : static_cast<unsigned long long>(~(-value - 1));
}
template<signed_mode Signed_Mode, typename T>
sfinae::enable_t<Signed_Mode == signed_mode::OFFSET_BINARY, unsigned long long> interpret_to(T value) {
  constexpr auto offset = 0b10000000ull << 8 * (sizeof(T) - 1);
  return static_cast<unsigned long long>(value + offset);
}

} // namespace detail
} // namespace upd