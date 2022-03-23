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
//
