//! \file

#pragma once

//! \brief Bind an object with static storage duration to a compile-time reference
#define K2O_CTREF(VALUE)                                                                                               \
  ::k2o::unevaluated<decltype(VALUE) &, VALUE> {}

namespace k2o {

//! \brief Holds a value without evaluating it
//!
//! Is is mainly useful for holding compile-time references to objects with static storage duration.
//!
//! \tparam T type of the held value
//! \tparam Value value of the held value
template<typename T, T Value>
struct unevaluated {
  using type = T;
};

} // namespace k2o
