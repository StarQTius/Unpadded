//! \file
//! \brief Basic type aliasing

#pragma once

#include <upd/type.hpp>

namespace k2o {

//! \brief Represent a byte of data
//! \detail
//!   The tuple class from the 'unpadded' library serializes a set of integers into a sequence of value of type
//!   'upd::byte_t'. For compatibility purpose, this type is brought into the namespace 'k2o' as 'byte_t'.
using upd::byte_t;

} // namespace k2o
