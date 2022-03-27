//! \file
//! \brief Private macro utilities

#define FWD(x) static_cast<decltype(x) &&>(x)
#define PACK(...) __VA_ARGS__
#define REQUIRE_CLASS(...) bool X = false, sfinae::require < X || (__VA_ARGS__) > = 0
