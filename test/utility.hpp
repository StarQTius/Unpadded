#pragma once

#include <unity.h>

#ifdef FWD
#error "Private macros have leaked"
#endif

extern "C" void setUp() {}
extern "C" void tearDown() {}

#define PACK(...) __VA_ARGS__
#define DECLVAL(...) std::declval<PACK(__VA_ARGS__)>()
#define DETECT(...) decltype(__VA_ARGS__, DECLVAL(int)){};

#define INTEGER DECLVAL(int &)

#define BYTE_PTR DECLVAL(upd::byte_t *)
#define READABLE DECLVAL(upd::byte_t (&)())
#define WRITABLE DECLVAL(void (&)(upd::byte_t))
#define REGISTRY DECLVAL(const volatile upd::byte_t &)
#define WRITABLE DECLVAL(void (&)(upd::byte_t))
