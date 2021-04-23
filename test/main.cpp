#include "main.hpp"

void setup() {
  UNITY_BEGIN();

  TEST_MESSAGE(upd::system::endianess == upd::system::Endianess::little ?
    "Platform uses little-endian" : "Platform uses big-endian");

  RUN_TEST(io_operation_write_and_interpret_little_endian);
  RUN_TEST(io_operation_write_and_interpret_big_endian);
  RUN_TEST(io_operation_set_and_get_little_endian);
  RUN_TEST(io_operation_set_and_get_big_endian);
  RUN_TEST(io_operation_iterate_unaligned_data);
  RUN_TEST(io_operation_iterate_unaligned_arguments);
  RUN_TEST(io_operation_access_raw_data);

  UNITY_END();
}

void loop() {}
