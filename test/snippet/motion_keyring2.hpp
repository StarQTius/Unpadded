#pragma once

#include <cstdint>

#include <upd/format.hpp>
#include <upd/keyring.hpp>
#include <upd/typelist.hpp>

struct my_callable {
  double factor;

  explicit my_callable(double factor) : factor{factor} {}

  void operator()(std::uint32_t);
};

void set_forward_speed(std::uint32_t);
auto set_left_steering_speed = [](std::uint32_t x) { /** implementation **/ };
extern my_callable set_right_steering_speed{1.5};

constexpr upd::keyring motion_keyring{upd::flist<set_forward_speed, set_left_steering_speed, set_right_steering_speed>,
                                      upd::little_endian,
                                      upd::two_complement};
