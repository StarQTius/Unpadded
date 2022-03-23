#include <k2o/key_base.hpp>

#include "utility.hpp"

using byte_t = uint8_t;

int function(int);

struct object_t {
  uint8_t a;
  uint16_t b, c;
};

struct object_extension_t {
  template<typename View_T>
  static void serialize(const object_t &o, View_T &view) {
    view = upd::make_tuple(o.a, o.b, o.c);
  }

  static object_t unserialize(uint8_t a, uint16_t b, uint16_t c) { return {a, b, c}; }
};
object_extension_t upd_extension(object_t *) { return {}; }

static void key_base_DO_serialize_arguments_EXPECT_correct_byte_sequence() {
  using namespace k2o;

  key_base<int(int, const int(&)[8], char)> k;
  int array[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
  auto t = upd::make_tuple(int{64}, array, char{16});
  byte_t dest_buf[t.size];

  int i = 0;
  t.invoke(k) >> [&](byte_t byte) { dest_buf[i++] = byte; };

  TEST_ASSERT_EQUAL_UINT8_ARRAY(t.begin(), dest_buf, t.size);
}

static void key_base_DO_serialize_arguments_with_parameter_EXPECT_correct_byte_sequence() {
  using namespace k2o;

  key_base<int(int, short, char), upd::endianess::LITTLE, upd::signed_mode::TWO_COMPLEMENT> k;
  auto dest_tuple = upd::make_tuple<int, short, char>(upd::little_endian, upd::two_complement);

  int i = 0;
  k(64, 32, 16) >> [&](byte_t byte) { *(dest_tuple.begin() + i++) = byte; };

  TEST_ASSERT_EQUAL_INT(64, dest_tuple.get<0>());
  TEST_ASSERT_EQUAL_INT(32, dest_tuple.get<1>());
  TEST_ASSERT_EQUAL_INT(16, dest_tuple.get<2>());
}

static void key_base_DO_unserialize_data_sequence_EXPECT_correct_value() {
  using namespace k2o;

  key_base<int()> k;
  auto t = upd::make_tuple(int{64});

  int i = 0;
  auto value = k << [&]() { return t[i++]; };

  TEST_ASSERT_EQUAL_INT(t.get<0>(), value);
}

static void key_base_DO_unserialize_data_sequence_with_parameter_EXPECT_correct_value() {
  using namespace k2o;

  key_base<int(int, short, char), upd::endianess::LITTLE, upd::signed_mode::TWO_COMPLEMENT> k;
  auto src_tuple = upd::make_tuple<>(upd::little_endian, upd::two_complement, int{64});

  int i = 0;
  auto result = k << [&]() { return *(src_tuple.begin() + i++); };

  TEST_ASSERT_EQUAL_INT(src_tuple.get<0>(), result);
}

static void key_base_DO_create_key_from_ftor_signature_EXPECT_key_holding_ftor_signature() {
  using namespace k2o;

  auto ftor = [](int x) { return x; };
  key_base<decltype(ftor)> k;

  int i = 0, j = 0;
  auto buf = upd::make_tuple<int>();
  k(64) >> [&](byte_t byte) { buf[i++] = byte; };
  auto result = k << [&]() { return buf[j++]; };

  TEST_ASSERT_EQUAL_INT(64, result);
}

static void key_base_DO_create_key_from_function_EXPECT_key_holding_function_signature_cpp17() {
#if __cplusplus >= 201703L
  using namespace k2o;

  auto k = make_key<function>(upd::builtin_endianess, upd::builtin_signed_mode);
  int i = 0, j = 0;
  byte_t buf[sizeof i];
  k(64) >> [&](byte_t byte) { buf[i++] = byte; };
  auto result = k << [&]() { return buf[j++]; };

  TEST_ASSERT_EQUAL_INT(64, result);
#endif // __cplusplus >= 201703L
}

static void key_base_DO_create_key_from_function_using_user_extended_type_EXPECT_correct_behaviour() {
  using namespace k2o;

  auto ftor = [](const object_t &x) { return x; };
  auto buf = upd::make_tuple<object_t>();
  key_base<decltype(ftor)> k;

  int i = 0, j = 0;
  k({0xa, 0xb, 0xc}) >> [&](byte_t byte) { buf[i++] = byte; };
  auto result = k << [&]() { return buf[j++]; };

  TEST_ASSERT_EQUAL_UINT8(result.a, 0xa);
  TEST_ASSERT_EQUAL_UINT16(result.b, 0xb);
  TEST_ASSERT_EQUAL_UINT16(result.c, 0xc);
}

static void key_base_DO_hook_a_callback_EXPECT_callback_receiving_correct_argument() {
  using namespace k2o;

  key_base<int()> k;
  auto t = upd::make_tuple(int{64});

  int i = 0;
  k.with_hook([](int value) { TEST_ASSERT_EQUAL_INT(64, value); })([&]() { return t[i++]; });
}

int main() {
  using namespace k2o;

#define KEY_BASE DECLVAL(key_base<int(int), upd::endianess::BUILTIN, upd::signed_mode::BUILTIN>)
#define KEY_WITH_HOOK DECLVAL(key_with_hook)
#define BYTE_PTR DECLVAL(upd::byte_t *)
#define READABLE DECLVAL(upd::byte_t (&)())
#define REGISTRY DECLVAL(const volatile upd::byte_t &)
#define INTEGER DECLVAL(int &)
#define WRITABLE DECLVAL(void (&)(upd::byte_t))

  DETECT(INTEGER = KEY_BASE.read_all(BYTE_PTR),
         INTEGER = KEY_BASE.read_all(READABLE),
         INTEGER = KEY_BASE << BYTE_PTR,
         INTEGER = KEY_BASE << READABLE,
         KEY_BASE(0).write_all(BYTE_PTR),
         KEY_BASE(0).write_all(WRITABLE),
         KEY_BASE(0) >> BYTE_PTR,
         KEY_BASE(0) >> WRITABLE,
         KEY_WITH_HOOK(BYTE_PTR),
         KEY_WITH_HOOK(READABLE));

  UNITY_BEGIN();
  RUN_TEST(key_base_DO_serialize_arguments_EXPECT_correct_byte_sequence);
  RUN_TEST(key_base_DO_serialize_arguments_with_parameter_EXPECT_correct_byte_sequence);
  RUN_TEST(key_base_DO_unserialize_data_sequence_EXPECT_correct_value);
  RUN_TEST(key_base_DO_unserialize_data_sequence_with_parameter_EXPECT_correct_value);
  RUN_TEST(key_base_DO_create_key_from_ftor_signature_EXPECT_key_holding_ftor_signature);
  RUN_TEST(key_base_DO_create_key_from_function_EXPECT_key_holding_function_signature_cpp17);
  RUN_TEST(key_base_DO_create_key_from_function_using_user_extended_type_EXPECT_correct_behaviour);
  RUN_TEST(key_base_DO_hook_a_callback_EXPECT_callback_receiving_correct_argument);
  return UNITY_END();
}
