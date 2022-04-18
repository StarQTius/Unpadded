//! \file

#pragma once

#include "detector.hpp"
#include "require.hpp"

#include "../def.hpp"

namespace upd {
namespace detail {

K2O_DETAIL_MAKE_DETECTOR(is_byte_iterator_impl, PACK(typename T), PACK(require_byte_iterator<T> = 0))

//! \brief Indicates if `T` is an iterator to a byte sequence
template<typename T>
struct is_byte_iterator : decltype(is_byte_iterator_impl<T>(0)) {};

} // namespace detail
} // namespace upd

#include "../undef.hpp" // IWYU pragma: keep
