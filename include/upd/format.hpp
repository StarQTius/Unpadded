#pragma once

#include <upd/detail/value_h.hpp>

#include "type.hpp"

/*!
  \file
  \brief Definitions for specifying data format
*/

namespace upd {

/*!
  \brief Used to specify endianess
*/
enum class endianess { BUILTIN, LITTLE, BIG };

/*!
  \brief Used to specify signed integer representation
*/
enum class signed_mode { BUILTIN, SIGNED_MAGNITUDE, ONE_COMPLEMENT, TWO_COMPLEMENT, OFFSET_BINARY };

#if __cplusplus >= 201703L
//! \brief (C++17) Token associated with 'endianess::BUILTIN'
constexpr value_h<endianess::BUILTIN> builtin_endianess;

//! \brief (C++17) Token associated with 'endianess::LITTLE'
constexpr value_h<endianess::LITTLE> little_endian;

//! \brief (C++17) Token associated with 'endianess::BIG'
constexpr value_h<endianess::BIG> big_endian;

//! \brief (C++17) Token associated with 'signed_mode::BUILTIN'
constexpr value_h<signed_mode::BUILTIN> builtin_signed_mode;

//! \brief (C++17) Token associated with 'signed_mode::SIGNED_MAGNITUDE'
constexpr value_h<signed_mode::SIGNED_MAGNITUDE> signed_magnitude;

//! \brief (C++17) Token associated with 'signed_mode::ONE_COMPLEMENT'
constexpr value_h<signed_mode::ONE_COMPLEMENT> one_complement;

//! \brief (C++17) Token associated with 'signed_mode::TWO_COMPLEMENT'
constexpr value_h<signed_mode::TWO_COMPLEMENT> two_complement;

//! \brief (C++17) Token associated with 'signed_mode::OFFSET_BINARY'
constexpr value_h<signed_mode::OFFSET_BINARY> offset_binary;
#endif // __cplusplus >= 201703L
} // namespace upd
