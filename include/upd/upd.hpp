#pragma once

#define UPD_FWD(x) static_cast<decltype(x) &&>(x)
#define UPD_PACK(...) __VA_ARGS__

namespace upd {
enum class endianess;
enum class signed_mode;

constexpr class {
public:
  struct {
    constexpr bool operator==(endianess) const { return false; }
    constexpr bool operator!=(endianess) const { return true; }
  } endianess;
  struct {
    constexpr bool operator==(signed_mode) const { return false; }
    constexpr bool operator!=(signed_mode) const { return true; }
  } signed_mode;
} platform_info;
} // namespace upd

template<typename>
struct upd_extension; // IWYU pragma: keep
