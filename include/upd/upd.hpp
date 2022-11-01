#pragma once

#define UPD_FWD(x) static_cast<decltype(x) &&>(x)
#define UPD_PACK(...) __VA_ARGS__

#if !defined(DOXYGEN)

#define UPD_REQUIRE(...) ::upd::detail::require<__VA_ARGS__> = 0
#define UPD_REQUIREMENT(NAME, ...) ::upd::detail::require_##NAME<__VA_ARGS__> = 0
#define UPD_REQUIRE_CLASS(...)                                                                                         \
  bool __Require_Class = false, ::upd::detail::require < __Require_Class || (__VA_ARGS__) > = 0

#else // !defined(DOXYGEN)

#define UPD_REQUIRE(...)
#define UPD_REQUIREMENT(...)
#define UPD_REQUIRE_CLASS(...)

#endif // !defined(DOXYGEN)

namespace upd {}

template<typename>
struct upd_extension; // IWYU pragma: keep
