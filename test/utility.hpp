#pragma once

#include "product.hpp"
#include <unity.h>
#include <upd/typelist.hpp>

#define MAKE_MULTIOPT(FUNCTION_NAME)                                                                                   \
  template<upd::endianess... Endianesses, upd::signed_mode... Signed_Modes>                                            \
  void FUNCTION_NAME##_multiopt(                                                                                       \
      upd::typelist_t<upd::typelist_t<endianess_token<Endianesses>, signed_mode_token<Signed_Modes>>...>) {            \
    using discard = int[];                                                                                             \
    (void)discard{(RUN_TEST((FUNCTION_NAME<Endianesses, Signed_Modes>)), 0)...};                                       \
  }

template<upd::endianess Endianess>
struct endianess_token {};
template<upd::signed_mode Signed_Mode>
struct signed_mode_token {};

constexpr auto every_options =
    typename product<upd::typelist_t<endianess_token<upd::endianess::LITTLE>, endianess_token<upd::endianess::BIG>>,
                     upd::typelist_t<signed_mode_token<upd::signed_mode::SIGNED_MAGNITUDE>,
                                     signed_mode_token<upd::signed_mode::ONES_COMPLEMENT>,
                                     signed_mode_token<upd::signed_mode::TWOS_COMPLEMENT>,
                                     signed_mode_token<upd::signed_mode::OFFSET_BINARY>>>::type{};
#if !defined(UPD_TEST_SETUP)
extern "C" void setUp() {}
extern "C" void tearDown() {}
#endif

int undefined_function(int);
