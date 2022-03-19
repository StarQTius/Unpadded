#pragma once

#include <unity.h>

#ifdef FWD
#error "Private macros have leaked"
#endif

extern "C" void setUp() {}
extern "C" void tearDown() {}
