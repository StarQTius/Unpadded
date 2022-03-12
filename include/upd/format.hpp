//! \file
//! \brief Definitions for specifying data format

#pragma once

#include "detail/value_h.hpp"

namespace upd {

//! \brief Used to specify endianess
enum class endianess { BUILTIN, LITTLE, BIG };

//! \brief Used to specify signed integer representation
enum class signed_mode { BUILTIN, SIGNED_MAGNITUDE, ONE_COMPLEMENT, TWO_COMPLEMENT, OFFSET_BINARY };

//! \brief
template<endianess Endianess>
struct endianess_h : detail::value_h<endianess, Endianess> {};

//! \brief
template<signed_mode Signed_Mode>
struct signed_mode_h : detail::value_h<signed_mode, Signed_Mode> {};

//! \brief Token associated with 'endianess::BUILTIN'
constexpr endianess_h<endianess::BUILTIN> builtin_endianess;

//! \brief Token associated with 'endianess::LITTLE'
constexpr endianess_h<endianess::LITTLE> little_endian;

//! \brief Token associated with 'endianess::BIG'
constexpr endianess_h<endianess::BIG> big_endian;

//! \brief Token associated with 'signed_mode::BUILTIN'
constexpr signed_mode_h<signed_mode::BUILTIN> builtin_signed_mode;

//! \brief Token associated with 'signed_mode::SIGNED_MAGNITUDE'
constexpr signed_mode_h<signed_mode::SIGNED_MAGNITUDE> signed_magnitude;

//! \brief Token associated with 'signed_mode::ONE_COMPLEMENT'
constexpr signed_mode_h<signed_mode::ONE_COMPLEMENT> one_complement;

//! \brief Token associated with 'signed_mode::TWO_COMPLEMENT'
constexpr signed_mode_h<signed_mode::TWO_COMPLEMENT> two_complement;

//! \brief Token associated with 'signed_mode::OFFSET_BINARY'
constexpr signed_mode_h<signed_mode::OFFSET_BINARY> offset_binary;

} // namespace upd
