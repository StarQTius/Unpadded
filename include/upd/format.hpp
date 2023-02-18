//! \file

#pragma once

#include "unevaluated.hpp"

namespace upd {

//! \brief Used to specify endianess for serialization and unserialization
enum class endianess { LITTLE, BIG };

//! \brief Used to specify signed integer representation for serialization and unserialization
enum class signed_mode { SIGNED_MAGNITUDE, ONE_COMPLEMENT, TWO_COMPLEMENT, OFFSET_BINARY };

//! \brief \ref<endianess> endianess enumerator holder for named parameters
template<endianess Endianess>
struct endianess_h : unevaluated<endianess, Endianess> {};

//! \brief \ref<signed_mode> signed_mode enumerator holder for named parameters
template<signed_mode Signed_Mode>
struct signed_mode_h : unevaluated<signed_mode, Signed_Mode> {};

//! \brief Token associated with endianess::LITTLE
constexpr endianess_h<endianess::LITTLE> little_endian;

//! \brief Token associated with endianess::BIG
constexpr endianess_h<endianess::BIG> big_endian;

//! \brief Token associated with signed_mode::SIGNED_MAGNITUDE
constexpr signed_mode_h<signed_mode::SIGNED_MAGNITUDE> signed_magnitude;

//! \brief Token associated with signed_mode::ONE_COMPLEMENT
constexpr signed_mode_h<signed_mode::ONE_COMPLEMENT> one_complement;

//! \brief Token associated with signed_mode::TWO_COMPLEMENT
constexpr signed_mode_h<signed_mode::TWO_COMPLEMENT> two_complement;

//! \brief Token associated with signed_mode::OFFSET_BINARY
constexpr signed_mode_h<signed_mode::OFFSET_BINARY> offset_binary;

} // namespace upd
