#pragma once

#include <cstdint>

#include <upd/format.hpp>
#include <upd/keyring.hpp>
#include <upd/typelist.hpp>
#include <upd/unevaluated.hpp>

void set_forward_speed(std::uint32_t);
void set_left_steering_speed(std::uint32_t);
void set_right_steering_speed(std::uint32_t);

constexpr auto motion_keyring = upd::make_keyring(upd::flist_t<UPD_CTREF(set_forward_speed),
                                                               UPD_CTREF(set_left_steering_speed),
                                                               UPD_CTREF(set_right_steering_speed)>{},
                                                  upd::little_endian,
                                                  upd::two_complement);
