#include <k2o/key.hpp>

#include "utility.hpp"

using byte_t = uint8_t;

static void key_DO_serialize_argument_EXPECT_correct_id_and_result() {
  using namespace k2o;

  key<16, int(int)> k;

  auto buf = upd::make_tuple(uint16_t{0}, int{0});
  size_t i = 0;
  k(64) >> [&](byte_t byte) { buf[i++] = byte; };

  TEST_ASSERT_EQUAL_INT(16, buf.get<0>());
  TEST_ASSERT_EQUAL_INT(64, buf.get<1>());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(key_DO_serialize_argument_EXPECT_correct_id_and_result);
  return UNITY_END();
}
