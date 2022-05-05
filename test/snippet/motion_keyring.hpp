#include <cassert>
#include <cstddef>

#include "motion_keyring1.hpp"

constexpr upd::byte_t to_slave[] = {0,
                                    20000 & 0xff,
                                    (20000 >> 8) & 0xff,
                                    (20000 >> 16) & 0xff,
                                    (20000 >> 24) & 0xff,
                                    1,
                                    10000 & 0xff,
                                    (10000 >> 8) & 0xff,
                                    (10000 >> 16) & 0xff,
                                    (10000 >> 24) & 0xff};

static void write_byte_to_master(upd::byte_t byte) { assert(false); }

static upd::byte_t read_byte_from_master() {
  static const upd::byte_t *p = to_slave;

  return *p++;
}

static void write_byte_to_slave(upd::byte_t byte) {
  static const upd::byte_t *p = to_slave;

  assert(*p++ == byte);
}

static upd::byte_t read_byte_from_slave() { assert(false); }

#if defined(CALLEE)

void on_byte_received();
void on_byte_sent();

static void do_stuff() {
  std::size_t ibyte_counter = 0;

  for (std::size_t ibyte_counter = 0; ibyte_counter < sizeof to_slave; ibyte_counter++)
    on_byte_received();
  on_byte_sent();

  std::exit(0);
}

#endif // defined(CALLEE)
