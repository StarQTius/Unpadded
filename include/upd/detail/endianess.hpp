//! \file
//! \brief Endianess portability support

#pragma once

#include <boost/type_traits.hpp>

#include "../format.hpp"
#include "../type.hpp"
#include "sfinae.hpp"

namespace upd {
namespace detail {

//! \brief Interpret a sequence of byte as an integer according to the provided endianess
template<typename T, endianess Endianess, sfinae::require<Endianess == endianess::BUILTIN> = 0>
T from_endianess(const byte_t *raw_data, size_t offset, size_t n) {
  T retval;
  memcpy(&retval, raw_data + offset, n);

  return retval;
}
template<typename T, endianess Endianess, sfinae::require<Endianess == endianess::LITTLE> = 0>
T from_endianess(const byte_t *raw_data, size_t offset, size_t n) {
  T retval = 0;
  size_t shift = 0;

  for (size_t i = 0; i < n; i++, shift += 8)
    retval |= static_cast<T>(raw_data[offset + i]) << shift;

  return retval;
}
template<typename T, endianess Endianess, sfinae::require<Endianess == endianess::BIG> = 0>
T from_endianess(const byte_t *raw_data, size_t offset, size_t n) {
  T retval = 0;
  size_t shift = 0;

  for (size_t i = 0; i < n; i++, shift += 8)
    retval |= static_cast<T>(raw_data[offset + (n - i - 1)]) << shift;

  return retval;
}

//! \brief Serialize an integer into a sequence of byte according to the provided endianess
template<endianess Endianess, typename T, sfinae::require<Endianess == endianess::BUILTIN> = 0>
void to_endianess(byte_t *raw_data, const T &x, size_t offset, size_t n) {
  memcpy(raw_data + offset, &x, n);
}
template<endianess Endianess, typename T, sfinae::require<Endianess == endianess::LITTLE> = 0>
void to_endianess(byte_t *raw_data, T x, size_t offset, size_t n) {
  for (size_t i = 0; i < n; i++, x >>= 8)
    raw_data[offset + i] = x & 0xff;
}
template<endianess Endianess, typename T, sfinae::require<Endianess == endianess::BIG> = 0>
void to_endianess(byte_t *raw_data, T x, size_t offset, size_t n) {
  for (size_t i = 0; i < n; i++, x >>= 8)
    raw_data[offset + (n - i - 1)] = x & 0xff;
}

} // namespace detail
} // namespace upd
