#include <cassert>
#include <cstdint>

#include <upd/format.hpp>
#include <upd/keyring.hpp>
#include <upd/typelist.hpp>

void f(std::uint8_t, std::uint16_t);

constexpr upd::keyring keyring{upd::flist<f>, upd::little_endian, upd::two_complement};

#include "../caller1"

void write_byte_to_slave(upd::byte_t byte) {
  static upd::byte_t buf[32], *p = buf;
  constexpr upd::byte_t expected[] = {0, 16, 64, 0, 0, 16, 64, 0};

  assert(p - buf < sizeof expected);
  assert(byte == expected[p - buf]);
  *p++ = byte;
}

int main() {

#include "../caller2"

  return 0;
}
