#include <upd/action.hpp>
#include <upd/buffered_dispatcher.hpp>

#include "motion_keyring.hpp"

static upd::double_buffered_dispatcher dispatcher{motion_keyring, upd::policy::any_action};

void set_forward_speed(std::uint32_t speed) {
  // function implementation
}

void set_left_steering_speed(std::uint32_t speed) {
  // function implementation
}

void set_right_steering_speed(std::uint32_t speed) {
  // function implementation
}

void on_byte_received() {
  // We read the incoming byte
  auto status = dispatcher.put(read_byte_from_master());

  switch (status) {
  // If a packet has been fully received, then the request has been fulfilled and the output buffer contains a response
  // packet
  case upd::packet_status::RESOLVED_PACKET:
    // We thus start to send the response packet to the master
    write_byte_to_master(dispatcher.get());
    break;
  // If a packet has been dropped, that means the received index is out of bound
  // At this point, the input buffer is empty
  // In that case, it is up to you to handle the issue, but it might be wise to stop reading the date in order to
  // prevent the dispatcher from reading garbage
  case upd::packet_status::DROPPED_PACKET:
    // Handling the error here
    break;
  default:
    break;
  }
}

void on_byte_sent() {
  // We keep sending the bytes of the response packet to the master
  // If the packet has been completely sent already, then this line has no effect
  write_byte_to_master(dispatcher.get());
}

int main() {
  while (true)
    do_stuff();
  return 0;
}
