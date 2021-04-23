#pragma once

#include "type.hpp"

/*!
  \file
  \brief Definitions for deducing platform specificity
*/

namespace upd {
namespace system {

/*!
  \brief Used to specify endianess
*/
enum class endianess { LITTLE, BIG };

//! \brief The value returned by deduce_endianess()
extern const endianess platform_endianess;

/*!
  \brief Deduce the current platform endianess
  \attention This function has yet to be fully tested.
  \return The deduced endianess
*/
inline endianess deduce_endianess() {
  unsigned short word = 0xff00;
  return (*reinterpret_cast<byte_t*>(&word) == 0xff ? endianess::BIG : endianess::LITTLE );
}

} // namespace system
} // namespace upd
