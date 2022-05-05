#pragma once

#include <cstdint>

#include <upd/format.hpp>
#include <upd/keyring.hpp>
#include <upd/typelist.hpp>
#include <upd/unevaluated.hpp>

void set_forward_speed(std::uint32_t);
void set_left_steering_speed(std::uint32_t);
void set_right_steering_speed(std::uint32_t);

constexpr auto motion_keyring =
    upd::make_keyring(upd::flist_t<unevaluated<decltype(set_forward_speed), set_forward_speed>,
                                   unevaluated<decltype(set_left_steering_speed), set_left_steering_speed>,
                                   unevaluated<decltype(set_right_steering_speed), set_right_steering_speed>>{},
                      upd::little_endian,
                      upd::two_complement);
