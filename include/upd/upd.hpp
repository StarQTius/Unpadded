#pragma once

#include "format.hpp" // IWYU pragma: keep

namespace upd {

#if defined(UPD_DEBUG)

constexpr auto release = false;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)                                                                                                \
  if (!(__VA_ARGS__)) {                                                                                                \
    throw std::exception{};                                                                                            \
  }

#else // defined(UPD_DEBUG)

constexpr auto release = true;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)

#endif // defined(UPD_DEBUG)

} // namespace upd

// NOLINTBEGIN

#define UPD_FWD(x) static_cast<decltype(x) &&>(x)
#define UPD_PACK(...) __VA_ARGS__
#define UPD_SCOPE_OPERATOR(LHS, RHS) LHS::RHS

namespace upd {

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
