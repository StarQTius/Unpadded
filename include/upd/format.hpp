#pragma once

#include "type.hpp"

/*!
  \file
  \brief Definitions for specifying data format
*/

namespace upd {

/*!
  \brief Used to specify endianess
*/
enum class endianess { LITTLE, BIG };

/*!
  \brief Used to specify signed integer representation
*/
enum class signed_mode { SIGNED_MAGNITUDE, ONE_COMPLEMENT, TWO_COMPLEMENT, OFFSET_BINARY };

} // namespace upd
