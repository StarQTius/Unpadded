//! \file

#pragma once

//! \brief Bind an object with static storage duration to a compile-time reference
//! \related unevaluated
#define UPD_CTREF(VALUE)                                                                                               \
  ::upd::unevaluated<decltype(VALUE) *, &VALUE> {}

namespace upd {

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

} // namespace upd
