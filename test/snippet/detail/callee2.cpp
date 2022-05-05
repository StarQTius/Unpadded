#include <cassert>
#include <cstdint>

#include <upd/dispatcher.hpp>
#include <upd/format.hpp>
#include <upd/keyring.hpp>
#include <upd/typelist.hpp>

std::uint8_t f(std::uint16_t x) { return x / 4; }

constexpr upd::keyring keyring{upd::flist<f>, upd::little_endian, upd::two_complement};

#include "../callee1"
#include "../callee2"

static bool complete = false;

void write_byte_to_master(upd::byte_t byte) {
  static upd::byte_t buf[32], *p = buf;

  *p++ = byte;

  if (p == buf + sizeof(std::uint8_t)) {
    auto key = keyring.get<f>();
    assert(key << buf == 128);
    complete = true;
  }
}

upd::byte_t read_byte_from_master() {
  auto key = keyring.get<f>();
  static upd::byte_t buf[32], *p = buf;

  if (p == buf)
    key(512) >> buf;

  return *p++;
}

int main() {
  processing_request();

  assert(complete);

  return 0;
}
