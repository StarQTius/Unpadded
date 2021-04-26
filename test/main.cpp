#include "main.hpp"

void setup() {
  UNITY_BEGIN();

  RUN_TEST(storage_write_and_interpret_unsigned_little_endian);
  RUN_TEST(storage_write_and_interpret_unsigned_big_endian);
  RUN_TEST(storage_set_and_get_unsigned_little_endian);
  RUN_TEST(storage_set_and_get_unsigned_big_endian);
  RUN_TEST(storage_iterate_unaligned_data);
  RUN_TEST(storage_iterate_unaligned_arguments);
  RUN_TEST(storage_access_raw_data);

  UNITY_END();
}

void loop() {}
