#pragma once

#include "format.hpp" // IWYU pragma: keep

// NOLINTBEGIN

#define UPD_FWD(x) static_cast<decltype(x) &&>(x)
#define UPD_PACK(...) __VA_ARGS__
#define UPD_SCOPE_OPERATOR(LHS, RHS) LHS::RHS

namespace upd {

#if !defined(UPD_NOEXCEPT)

#define UPD_NOEXCEPT noexcept

#endif // !defined(UPD_NOEXCEPT)

#if !defined(UPD_ASSERT)

#define UPD_ASSERT(...)

constexpr auto release = true;

#else // !defined(UPD_ASSERT)

constexpr auto release = false;

#endif // !defined(UPD_ASSERT)

//! \brief Contains the platform-specific information provided by the user
//!
//! `platform_info.endianess` equals `UPD_PLATFORM_ENDIANESS`.
//! `platform_info.signed_mode` equals `UPD_PLATFORM_SIGNED_MODE`.
constexpr struct {
#if defined(UPD_PLATFORM_ENDIANESS)
  upd::endianess endianess = UPD_SCOPE_OPERATOR(upd::endianess, UPD_PLATFORM_ENDIANESS);
#else  // defined(UPD_PLATFORM_ENDIANESS)
  struct {
    constexpr bool operator==(endianess) const { return false; }
    constexpr bool operator!=(endianess) const { return true; }
  } endianess;
#endif // defined(UPD_PLATFORM_ENDIANESS)

#if defined(UPD_PLATFORM_SIGNED_MODE)
  upd::signed_mode signed_mode = UPD_SCOPE_OPERATOR(upd::signed_mode, UPD_PLATFORM_SIGNED_MODE);
#else  // defined(UPD_PLATFORM_SIGNED_MODE)
  struct {
    constexpr bool operator==(signed_mode) const { return false; }
    constexpr bool operator!=(signed_mode) const { return true; }
  } signed_mode;
#endif // defined(UPD_PLATFORM_SIGNED_MODE)
} platform_info;

} // namespace upd

template<typename>
struct upd_extension; // IWYU pragma: keep

namespace upd::detail {};

// NOLINTEND
