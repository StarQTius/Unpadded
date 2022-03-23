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
  using namespace k2o;

#define KEY DECLVAL(key<0, int(int), upd::endianess::BUILTIN, upd::signed_mode::BUILTIN>)
#define KEY_WITH_HOOK DECLVAL(key_with_hook)
#define BYTE_PTR DECLVAL(upd::byte_t *)
#define READABLE DECLVAL(upd::byte_t (&)())
#define INTEGER DECLVAL(int &)
#define WRITABLE DECLVAL(void (&)(upd::byte_t))

  DETECT(INTEGER = KEY.read_all(BYTE_PTR),
         INTEGER = KEY.read_all(READABLE),
         INTEGER = KEY << BYTE_PTR,
         INTEGER = KEY << READABLE,
         KEY(0).write_all(BYTE_PTR),
         KEY(0).write_all(WRITABLE),
         KEY(0) >> BYTE_PTR,
         KEY(0) >> WRITABLE,
         KEY_WITH_HOOK(BYTE_PTR),
         KEY_WITH_HOOK(READABLE));

  UNITY_BEGIN();
  RUN_TEST(key_DO_serialize_argument_EXPECT_correct_id_and_result);
  return UNITY_END();
}
