#pragma once

#include <boost/mp11.hpp>
#include <unity.h>

#ifdef FWD
#error "Private macros have leaked"
#endif

#define PACK(...) __VA_ARGS__
#define DECLVAL(...) std::declval<PACK(__VA_ARGS__)>()
#define DETECT(...) decltype(__VA_ARGS__, DECLVAL(int)){};

#define INTEGER DECLVAL(int &)

#define BYTE_PTR DECLVAL(upd::byte_t *)
#define READABLE DECLVAL(upd::byte_t (&)())
#define WRITABLE DECLVAL(void (&)(upd::byte_t))
#define REGISTRY DECLVAL(volatile upd::byte_t &)
#define WRITABLE DECLVAL(void (&)(upd::byte_t))
#define FUNCTOR undefined_function

#define MAKE_MULTIOPT(FUNCTION_NAME)                                                                                   \
  template<upd::endianess... Endianesses, upd::signed_mode... Signed_Modes>                                            \
  void FUNCTION_NAME##_multiopt(                                                                                       \
      boost::mp11::mp_list<options_token<endianess_token<Endianesses>, signed_mode_token<Signed_Modes>>...>) {         \
    using discard = int[];                                                                                             \
    discard{(RUN_TEST((FUNCTION_NAME<Endianesses, Signed_Modes>)), 0)...};                                             \
  }

template<typename, typename>
struct options_token;
template<upd::endianess Endianess>
struct endianess_token {};
template<upd::signed_mode Signed_Mode>
struct signed_mode_token {};

constexpr auto every_options =
    boost::mp11::mp_product<options_token,
                            boost::mp11::mp_list<endianess_token<upd::endianess::BUILTIN>,
                                                 endianess_token<upd::endianess::LITTLE>,
                                                 endianess_token<upd::endianess::BIG>>,
                            boost::mp11::mp_list<signed_mode_token<upd::signed_mode::BUILTIN>,
                                                 signed_mode_token<upd::signed_mode::SIGNED_MAGNITUDE>,
                                                 signed_mode_token<upd::signed_mode::ONE_COMPLEMENT>,
                                                 signed_mode_token<upd::signed_mode::TWO_COMPLEMENT>,
                                                 signed_mode_token<upd::signed_mode::OFFSET_BINARY>>>{};

extern "C" void setUp() {}
extern "C" void tearDown() {}

int undefined_function(int);
