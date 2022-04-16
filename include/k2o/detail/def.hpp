//! \file

#define FWD(x) static_cast<decltype(x) &&>(x)
#define PACK(...) __VA_ARGS__

#if !defined(DOXYGEN)

#define REQUIRE(...) detail::require<__VA_ARGS__> = 0
#define REQUIREMENT(NAME, ...) detail::require_##NAME<__VA_ARGS__> = 0
#define REQUIRE_CLASS(...) bool __Require_Class = false, detail::require < __Require_Class || (__VA_ARGS__) > = 0

#else // !defined(DOXYGEN)

#define REQUIRE(...)
#define REQUIREMENT(...)
#define REQUIRE_CLASS(...)

#endif // !defined(DOXYGEN_DEF_HPP)
