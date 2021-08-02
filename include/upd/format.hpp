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

//! \brief Token associated with 'endianess::BUILTIN'
constexpr value_h<endianess, endianess::BUILTIN> builtin_endianess;

//! \brief Token associated with 'endianess::LITTLE'
constexpr value_h<endianess, endianess::LITTLE> little_endian;

//! \brief Token associated with 'endianess::BIG'
constexpr value_h<endianess, endianess::BIG> big_endian;

//! \brief Token associated with 'signed_mode::BUILTIN'
constexpr value_h<signed_mode, signed_mode::BUILTIN> builtin_signed_mode;

//! \brief Token associated with 'signed_mode::SIGNED_MAGNITUDE'
constexpr value_h<signed_mode, signed_mode::SIGNED_MAGNITUDE> signed_magnitude;

//! \brief Token associated with 'signed_mode::ONE_COMPLEMENT'
constexpr value_h<signed_mode, signed_mode::ONE_COMPLEMENT> one_complement;

//! \brief Token associated with 'signed_mode::TWO_COMPLEMENT'
constexpr value_h<signed_mode, signed_mode::TWO_COMPLEMENT> two_complement;

//! \brief Token associated with 'signed_mode::OFFSET_BINARY'
constexpr value_h<signed_mode, signed_mode::OFFSET_BINARY> offset_binary;
} // namespace upd
