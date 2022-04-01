#include <k2o/key.hpp>

#include "utility.hpp"

using byte_t = uint8_t;

static void key_DO_serialize_argument_EXPECT_correct_id_and_result() {
  using namespace k2o;

  key<uint8_t, 16, int(int)> k;

  auto buf = upd::make_tuple(decltype(k)::index_t{0}, int{0});
  size_t i = 0;
  k(64) >> [&](byte_t byte) { buf[i++] = byte; };

  TEST_ASSERT_EQUAL_INT(16, buf.get<0>());
  TEST_ASSERT_EQUAL_INT(64, buf.get<1>());
}

#define KEY DECLVAL(key<uint8_t, 0, int(int), upd::endianess::BUILTIN, upd::signed_mode::BUILTIN>)

int main() {
  using namespace k2o;

  DETECT(INTEGER = KEY.read_all(BYTE_PTR),
         INTEGER = KEY.read_all(READABLE),
         INTEGER = KEY << BYTE_PTR,
         INTEGER = KEY << READABLE,
         KEY(0).write_all(BYTE_PTR),
         KEY(0).write_all(WRITABLE),
         KEY(0) >> BYTE_PTR,
         KEY(0) >> WRITABLE);

  UNITY_BEGIN();
  RUN_TEST(key_DO_serialize_argument_EXPECT_correct_id_and_result);
  return UNITY_END();
}
