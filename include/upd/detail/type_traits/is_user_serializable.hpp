//! \file

#pragma once

#include "detector.hpp"

#include "../def.hpp"

namespace upd {
namespace detail {

K2O_DETAIL_MAKE_DETECTOR(is_user_serializable_impl,
                         PACK(typename T),
                         PACK(typename = decltype(upd_extension((T *)nullptr))))

//! \brief Indicates whether `T` has a user-provided extension (i.e. `upd_extension((T*) nullptr)` is well-formed)
template<typename T>
struct is_user_serializable : decltype(is_user_serializable_impl<T>(0)) {};

} // namespace detail
} // namespace upd

#include "../undef.hpp"
