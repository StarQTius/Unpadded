#pragma once

/*!
  \file
  \brief Signed integer representation functions
  \details
    This header provides function for encoding and decoding signed values in several signed number representation. The
    representation do not have to be supported by the platform. When a signed number is encoded with any of the
    following encoding function, the result is an unsigned integer. The bitwise representation of that integer is the
    same as the bitwise representation as a signed integer of the same size expressed in the provided signed
    representation.
*/

namespace upd {
namespace detail {

/*!
*/
template<typename T>
T interpret_from_signed_magnitude(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  constexpr auto magnitude_mask = (~0ull >> 8 * (sizeof(value) - sizeof(T))) ^ sign_mask;
  return value & sign_mask ? -static_cast<T>(value & magnitude_mask) : static_cast<T>(value);
}

/*!
*/
template<typename T>
unsigned long long interpret_to_signed_magnitude(T value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value >= 0 ? static_cast<unsigned long long>(value) : static_cast<unsigned long long>(-value) | sign_mask;
}

/*!
  \brief Interpret an unsigned integer as a signed integer represented with one's complement
  \param value The unsigned integer to interpret
  \return A signed integer which is represented by value in one's complement
*/
template<typename T>
T interpret_from_one_complement(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value & sign_mask ? -static_cast<T>(~value) : static_cast<T>(value);
}

/*!
  \brief Interpret a signed integer as a unsigned integer representing it with one's complement
  \param value The signed integer to interpret
  \return A unsigned integer representing value in one's complement
*/
template<typename T>
unsigned long long interpret_to_one_complement(T value) {
  return value >= 0 ? static_cast<unsigned long long>(value) : ~static_cast<unsigned long long>(-value);
}

/*!
  \brief Interpret an unsigned integer as a signed integer represented with two's complement
  \param value The unsigned integer to interpret
  \return A signed integer which is represented by value in two's complement
*/
template<typename T>
T interpret_from_two_complement(unsigned long long value) {
  constexpr auto sign_mask = 0b10000000ull << 8 * (sizeof(T) - 1);
  return value & sign_mask ? -static_cast<T>(~value + 1) : static_cast<T>(value);
}

/*!
  \brief Interpret an unsigned integer as a signed integer represented with two's complement
  \param value The unsigned integer to interpret
  \return A signed integer which is represented by value in two's complement
*/
template<typename T>
unsigned long long interpret_to_two_complement(T value) {
  return value >= 0 ? static_cast<unsigned long long>(value) : static_cast<unsigned long long>(~(-value - 1));
}

} // namespace detail
} // namespace upd
