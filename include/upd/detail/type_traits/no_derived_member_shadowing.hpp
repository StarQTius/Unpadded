//! \file

#pragma once

namespace upd {
namespace detail {

//! \brief Prevent a base member from hiding a derived member when using the `using` keyword
//!
//! To make it work, just add it as an optional parameter (`void f(int x, char y, no_derived_member_shadowing = 0` for
//! example).
struct no_derived_member_shadowing {
  no_derived_member_shadowing(int) {}
};

} // namespace detail
} // namespace upd
