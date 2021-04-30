#include "main.hpp"

void setup() {
  UNITY_BEGIN();

  upd::unaligned_data<16> unaligned_data{upd::endianess::LITTLE, upd::signed_mode::TWO_COMPLEMENT};
  unaligned_data.write(0xabc, 0);

  char msg[16];
  snprintf(msg, 16, "%x", unaligned_data.interpret_as<int16_t>(0));
  TEST_MESSAGE(msg);

  RUN_TEST((storage_write_and_interpret<uint16_t, 0xabc>));
  RUN_TEST((storage_write_and_interpret<int16_t, 0xabc>));
  RUN_TEST((storage_write_and_interpret<int16_t, -0xabc>));
  RUN_TEST((storage_write_and_interpret<uint64_t, 0xabc>));
  RUN_TEST((storage_write_and_interpret<int64_t, 0xabc>));
  RUN_TEST((storage_write_and_interpret<int64_t, -0xabc>));

  RUN_TEST((storage_set_and_get<2, 0xabc, char, int, unsigned short, long, unsigned long, bool>));
  RUN_TEST((storage_set_and_get<1, 0xabc, char, int, unsigned short, long, unsigned long, bool>));
  RUN_TEST((storage_set_and_get<1, -0xabc, char, int, unsigned short, long, unsigned long, bool>));
  RUN_TEST((storage_set_and_get<4, 0xabc, char, int, unsigned short, long, unsigned long, bool>));
  RUN_TEST((storage_set_and_get<3, 0xabc, char, int, unsigned short, long, unsigned long, bool>));
  RUN_TEST((storage_set_and_get<3, -0xabc, char, int, unsigned short, long, unsigned long, bool>));

  RUN_TEST(storage_iterate_unaligned_data);
  RUN_TEST(storage_iterate_unaligned_arguments);
  RUN_TEST(storage_access_raw_data);

  UNITY_END();
}

void loop() {}
