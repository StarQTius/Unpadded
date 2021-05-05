#pragma once

#include "ct_magic.hpp"

/*!
  \file
  \brief Concept utility header for storage classes
*/

namespace upd {
namespace concept {

/*!
  \brief Utility class for SFINAE to check if a type is an unsigned integer
*/
template<typename T, typename U = void>
using require_unsigned_integer = ctm::enable_t<ctm::category::unsigned_integer.has<T>(), U>;

/*!
  \brief Utility class for SFINAE to check if a type is an signed integer
*/
template<typename T, typename U = void>
using require_signed_integer = ctm::enable_t<ctm::category::signed_integer.has<T>(), U>;

/*!
  \brief Utility class for SFINAE to check if a type is an array type
*/
template<typename T, typename U = void>
using require_array = ctm::enable_t<ctm::is_array<T>::value, U>;

} // namespace concept
} // namespace upd
