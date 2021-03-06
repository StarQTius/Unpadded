//! \file

#pragma once

#include <cstddef>
#include <cstring>

#include "../format.hpp"
#include "../type.hpp"
#include "type_traits/require.hpp"

namespace upd {
namespace detail {

//! \brief Interpret a sequence of byte as an integer according to the provided endianess
template<typename T, endianess Endianess, require<Endianess == endianess::BUILTIN> = 0>
T from_endianess(const byte_t *raw_data, std::size_t n) {
  auto retval = T(0);
  memcpy(&retval, raw_data, n);

  return retval;
}
template<typename T, endianess Endianess, require<Endianess == endianess::LITTLE> = 0>
T from_endianess(const byte_t *raw_data, std::size_t n) {
  auto retval = T(0);
  std::size_t shift = 0;

  for (std::size_t i = 0; i < n; i++, shift += 8) {
    retval = T(retval | (T(raw_data[i]) << shift));
  }

  return retval;
}
template<typename T, endianess Endianess, require<Endianess == endianess::BIG> = 0>
T from_endianess(const byte_t *raw_data, std::size_t n) {
  auto retval = T(0);
  unsigned int shift = 0;

  for (std::size_t i = 0; i < n; i++, shift += 8)
    retval = T(retval | T(raw_data[(n - i - 1)]) << shift);

  return retval;
}

//! \brief Serialize an integer into a sequence of byte according to the provided endianess
template<endianess Endianess, typename T, require<Endianess == endianess::BUILTIN> = 0>
void to_endianess(byte_t *raw_data, const T &x, std::size_t n) {
  memcpy(raw_data, &x, n);
}
template<endianess Endianess, typename T, require<Endianess == endianess::LITTLE> = 0>
void to_endianess(byte_t *raw_data, T x, std::size_t n) {
  for (std::size_t i = 0; i < n; i++, x = T(x >> 8))
    raw_data[i] = x & 0xff;
}
template<endianess Endianess, typename T, require<Endianess == endianess::BIG> = 0>
void to_endianess(byte_t *raw_data, T x, std::size_t n) {
  for (std::size_t i = 0; i < n; i++, x = T(x >> 8))
    raw_data[(n - i - 1)] = x & 0xff;
}

} // namespace detail
} // namespace upd
