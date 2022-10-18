//! \file

#pragma once

#include "../../upd.hpp"
#include "detector.hpp"

namespace upd {
namespace detail {

UPD_DETAIL_MAKE_DETECTOR(is_user_serializable_impl,
                         UPD_PACK(typename T),
                         UPD_PACK(typename = decltype(upd_extension((T *)nullptr))))

//! \brief Indicates whether `T` has a user-provided extension (i.e. `upd_extension((T*) nullptr)` is well-formed)
template<typename T>
struct is_user_serializable : decltype(is_user_serializable_impl<T>(0)) {};

} // namespace detail
} // namespace upd
