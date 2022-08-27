//! \file

#pragma once

#include "../../upd.hpp"
#include "detector.hpp"
#include "require.hpp"

namespace upd {
namespace detail {

UPD_DETAIL_MAKE_DETECTOR(is_input_byte_iterator_impl,
                         UPD_PACK(typename T),
                         UPD_PACK(require_input_byte_iterator<T> = 0))
UPD_DETAIL_MAKE_DETECTOR(is_output_byte_iterator_impl,
                         UPD_PACK(typename T),
                         UPD_PACK(require_output_byte_iterator<T> = 0))

//! \brief Check if `T` is an input byte iterator
template<typename T>
struct is_input_byte_iterator : decltype(is_input_byte_iterator_impl<T>(0)) {};

//! \brief Check if `T` is an output byte iterator
template<typename T>
struct is_output_byte_iterator : decltype(is_output_byte_iterator_impl<T>(0)) {};

} // namespace detail
} // namespace upd
