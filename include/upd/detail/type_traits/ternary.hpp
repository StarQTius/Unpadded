//! \file

#pragma once

namespace k2o {
namespace detail {

//! \name
//! \brief `ternary_t` is an alias for `True_T` is `Condition` evaluates to `true`, `False_T` otherwise
//! @{

template<bool Condition, typename True_T, typename>
struct ternary {
  using type = True_T;
};
template<typename True_T, typename False_T>
struct ternary<false, True_T, False_T> {
  using type = False_T;
};
template<bool Condition, typename True_T, typename False_T>
using ternary_t = typename ternary<Condition, True_T, False_T>::type;

//! @}

} // namespace detail
} // namespace k2o
