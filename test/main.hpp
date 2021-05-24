#if defined(UT_ONLY)

#define INTEGRATION_TEST TEST_IGNORE()

void setup();
int main() { setup(); return 0; }

#else // defined(UT_ONLY)

#define INTEGRATION_TEST
#include <Arduino.h>

#endif // defined(UT_ONLY)

#include "unity.h"

extern "C" void setUp() {}
extern "C" void tearDown() {}

#include <cstring>

#include "storage_ut.hpp"

constexpr auto every_options = boost::mp11::mp_product<
  options_h,
  boost::mp11::mp_list<
    endianess_h<upd::endianess::BUILTIN>,
    endianess_h<upd::endianess::LITTLE>,
    endianess_h<upd::endianess::BIG>>,
  boost::mp11::mp_list<
    signed_mode_h<upd::signed_mode::SIGNED_MAGNITUDE>,
    signed_mode_h<upd::signed_mode::ONE_COMPLEMENT>,
    signed_mode_h<upd::signed_mode::TWO_COMPLEMENT>,
    signed_mode_h<upd::signed_mode::OFFSET_BINARY>
  >
>{};
