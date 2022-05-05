#include "motion_keyring.hpp"

int main() {
  auto set_forward_speed_k = motion_keyring.get<set_forward_speed>();
  auto set_left_steering_speed_k = motion_keyring.get<set_left_steering_speed>();

  set_forward_speed_k(20000).write_all(write_byte_to_slave);
  set_left_steering_speed_k(10000).write_all(write_byte_to_slave);

  return 0;
}
