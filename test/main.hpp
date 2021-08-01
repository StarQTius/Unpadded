#pragma once

#if defined(UT_ONLY)

#define INTEGRATION_TEST TEST_IGNORE()

void setup();
int main() {
  setup();
  return 0;
}

#else // defined(UT_ONLY)

#define INTEGRATION_TEST

#endif // defined(UT_ONLY)

#include "unity.h"

#include "key.hpp"
#include "order.hpp"

extern "C" void setUp() {}
extern "C" void tearDown() {}
