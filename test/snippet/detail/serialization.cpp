#include <cassert>
#include <cstdint>

#include <upd/tuple.hpp>

template<typename... Args>
static std::uint16_t calculate_crc(Args...) {
  return 0xabcd;
}

void write_byte_to_device(upd::byte_t x) {
  static upd::byte_t buf[] = {0xff, 0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0x40, 0x0, 0x0, 0x0, 0x20, 0xab, 0xcd}, *p = buf;

  assert(*p++ == x);
}

upd::byte_t read_byte_from_device() {
  static upd::byte_t buf[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x0, 0x40, 0xab, 0xcd}, *p = buf;

  return *p++;
}

static std::int32_t f() {
#include "../serialization1"
}

#include "../serialization2"

int main() {
  assert(f() == -128 + 64);

  object_t o{16, 32, 64}, _o = upd::get<0>(upd::tuple{upd::little_endian, upd::twos_complement, o});

  assert(o.x == _o.x);
  assert(o.y == _o.y);
  assert(o.z == _o.z);
}
