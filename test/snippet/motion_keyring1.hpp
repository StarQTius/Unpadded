#pragma once

#include <cstdint>

#include <upd/format.hpp>
#include <upd/keyring.hpp>
#include <upd/typelist.hpp>

void set_forward_speed(std::uint32_t);
void set_left_steering_speed(std::uint32_t);
void set_right_steering_speed(std::uint32_t);

constexpr upd::keyring motion_keyring{upd::flist<set_forward_speed, set_left_steering_speed, set_right_steering_speed>,
                                      upd::little_endian,
                                      upd::two_complement};
