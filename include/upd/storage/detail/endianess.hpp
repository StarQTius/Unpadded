#pragma once

#include "boost/type_traits.hpp"

#include "upd/type.hpp"
#include "upd/format.hpp"
#include "upd/storage/concept.hpp"

/*!
  \file
*/

namespace upd {
namespace detail {

/*!
*/
template<typename T, endianess Endianess>
concept::enable_t<Endianess == endianess::LITTLE, T>
interpret_with_endianess(const byte_t* raw_data, size_t offset, size_t n) {
  T retval = 0;
  size_t shift = 0;

  for (size_t i = 0; i < n; i++, shift += 8)
    retval |= static_cast<T>(raw_data[offset + i]) << shift;

  return retval;
}

/*!
*/
template<typename T, endianess Endianess>
concept::enable_t<Endianess == endianess::BIG, T>
interpret_with_endianess(const byte_t* raw_data, size_t offset, size_t n) {
  T retval = 0;
  size_t shift = 0;

  for (size_t i = 0; i < n; i++, shift += 8)
    retval |= static_cast<T>(raw_data[offset + (n - i - 1)]) << shift;

  return retval;
}

/*!
*/
template<endianess Endianess, typename T>
concept::enable_t<Endianess == endianess::LITTLE>
write_with_endianess(byte_t* raw_data, T x, size_t offset, size_t n) {
  for (size_t i = 0; i < n; i++, x >>= 8)
    raw_data[offset + i] = x & 0xff;
}

/*!
*/
template<endianess Endianess, typename T>
concept::enable_t<Endianess == endianess::BIG>
write_with_endianess(byte_t* raw_data, T x, size_t offset, size_t n) {
  for (size_t i = 0; i < n; i++, x >>= 8)
    raw_data[offset + (n - i - 1)] = x & 0xff;
}

} // namespace detail
} // namespace upd
