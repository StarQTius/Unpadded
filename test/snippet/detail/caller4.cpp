#include <cassert>
#include <cstdint>

#include <upd/format.hpp>
#include <upd/keyring.hpp>
#include <upd/typelist.hpp>

std::uint8_t f(std::uint16_t);

constexpr upd::keyring keyring{upd::flist<f>, upd::little_endian, upd::two_complement};

#include "../caller1"

void write_byte_to_slave(upd::byte_t byte) {
  static upd::byte_t buf[32], *p = buf;
  constexpr upd::byte_t expected[] = {0, 64, 0};

  assert(p - buf < sizeof expected);
  assert(byte == expected[p - buf]);
  *p++ = byte;
}

upd::byte_t read_byte_from_slave() {
  static upd::byte_t buf[] = {48}, *p = buf;

  return *p++;
}

int main() {

#include "../caller4"

  assert(x == 48);

  return 0;
}
