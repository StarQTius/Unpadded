//! \file

#pragma once

#include <type_traits>

#include "detector.hpp"

#include "../def.hpp"

namespace k2o {
namespace detail {

K2O_DETAIL_MAKE_DETECTOR(
    has_signature_impl,
    PACK(typename F, typename R, typename... Args),
    PACK(typename = typename std::enable_if<
             std::is_convertible<decltype(std::declval<F>()(std::declval<Args>()...)), R>::value>::type))

//! \name
//! \brief Indicates whether the instances of `F` can be invoked as they were functions of type `R(Args...)`
//! @{

template<typename, typename>
struct has_signature : std::false_type {};
template<typename F, typename R, typename... Args>
struct has_signature<F, R(Args...)> : decltype(has_signature_impl<F, R, Args...>(0)) {};

//! @}

} // namespace detail
} // namespace k2o

#include "../undef.hpp" // IWYU pragma: keep
