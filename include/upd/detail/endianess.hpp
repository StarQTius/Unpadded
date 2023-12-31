//! \file

#pragma once

#include <cstddef>
#include <cstring>

#include "../format.hpp"
#include "../type.hpp"
#include "../upd.hpp"
#include "type_traits/require.hpp"

// NOLINTBEGIN

namespace upd {
namespace detail {

//! \brief Platform-agnostic implementations of `from_endianess`
template<typename T, endianess Endianess, require<Endianess == endianess::LITTLE> = 0>
T from_endianess_impl(const byte_t *raw_data, std::size_t n) {
  auto retval = T(0);
  std::size_t shift = 0;

  for (std::size_t i = 0; i < n; i++, shift += 8) {
    retval = T(retval | (T(raw_data[i]) << shift));
  }

  return retval;
}
template<typename T, endianess Endianess, require<Endianess == endianess::BIG> = 0>
T from_endianess_impl(const byte_t *raw_data, std::size_t n) {
  auto retval = T(0);
  unsigned int shift = 0;

  for (std::size_t i = 0; i < n; i++, shift += 8)
    retval = T(retval | T(raw_data[(n - i - 1)]) << shift);

  return retval;
}

//! \brief Interpret a sequence of byte as an integer according to the provided endianess
template<typename T, endianess Endianess, UPD_REQUIRE(platform_info.endianess == Endianess)>
T from_endianess(const byte_t *raw_data, std::size_t n) {
  auto retval = T(0);
  memcpy(&retval, raw_data, n);

  return retval;
}
template<typename T, endianess Endianess, UPD_REQUIRE(platform_info.endianess != Endianess)>
T from_endianess(const byte_t *raw_data, std::size_t n) {
  return from_endianess_impl<T, Endianess>(raw_data, n);
}

//! \brief Platform-agnostic implementations of `to_endianess`
template<endianess Endianess, typename T, require<Endianess == endianess::LITTLE> = 0>
void to_endianess_impl(byte_t *raw_data, T x, std::size_t n) {
  for (std::size_t i = 0; i < n; i++, x = T(x >> 8))
    raw_data[i] = x & 0xff;
}
template<endianess Endianess, typename T, require<Endianess == endianess::BIG> = 0>
void to_endianess_impl(byte_t *raw_data, T x, std::size_t n) {
  for (std::size_t i = 0; i < n; i++, x = T(x >> 8))
    raw_data[(n - i - 1)] = x & 0xff;
}

//! \brief Serialize an integer into a sequence of byte according to the provided endianess
template<endianess Endianess, typename T, UPD_REQUIRE(platform_info.endianess == Endianess)>
void to_endianess(byte_t *raw_data, const T &x, std::size_t n) {
  memcpy(raw_data, &x, n);
}
template<endianess Endianess, typename T, UPD_REQUIRE(platform_info.endianess != Endianess)>
void to_endianess(byte_t *raw_data, const T &x, std::size_t n) {
  to_endianess_impl<Endianess, T>(raw_data, x, n);
}

} // namespace detail
} // namespace upd

// NOLINTEND
