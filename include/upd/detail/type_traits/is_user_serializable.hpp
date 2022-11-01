//! \file

#pragma once

#include <type_traits>

#include "../../upd.hpp"
#include "detector.hpp"

namespace upd {
namespace detail {

UPD_DETAIL_MAKE_DETECTOR(is_not_user_serializable_impl,
                         UPD_PACK(typename T),
                         UPD_PACK(typename = decltype(upd_extension<T>{})))

//! \brief Indicates whether `T` has a user-provided extension
template<typename T>
struct is_user_serializable : std::integral_constant<bool, decltype(is_not_user_serializable_impl<T>(0))::value> {};

} // namespace detail
} // namespace upd
