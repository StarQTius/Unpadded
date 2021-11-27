#pragma once

#include <boost/mp11.hpp>

// To ensure that every tested header is self sufficient, upd/format.hpp is not included here.

template<upd::endianess Endianess>
struct endianess_token {};

template<upd::signed_mode Signed_Mode>
struct signed_mode_token {};

template<typename, typename>
struct options_token;

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct options_token<endianess_token<Endianess>, signed_mode_token<Signed_Mode>> {};

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

#define MAKE_MULTIOPT(FUNCTION_NAME)                                                                                   \
  template<upd::endianess... Endianesses, upd::signed_mode... Signed_Modes>                                            \
  void FUNCTION_NAME##_multiopt(                                                                                       \
      boost::mp11::mp_list<options_token<endianess_token<Endianesses>, signed_mode_token<Signed_Modes>>...>) {         \
    using discard = int[];                                                                                             \
    discard{(RUN_TEST((FUNCTION_NAME<Endianesses, Signed_Modes>)), 0)...};                                             \
  }
