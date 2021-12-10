#include "key.hpp"

using byte_t = uint8_t;

int function(int);

void key_DO_serialize_arguments_EXPECT_correct_byte_sequence() {
  using namespace k2o;

  key<int(int, const int(&)[8], char)> k;
  int array[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
  auto t = upd::make_tuple(int{64}, array, char{16});
  byte_t dest_buf[t.size];

  int i = 0;
  t.invoke(k) >> [&](byte_t byte) { dest_buf[i++] = byte; };

  TEST_ASSERT_EQUAL_UINT8_ARRAY(t.begin(), dest_buf, t.size);
}

void key_DO_serialize_arguments_with_parameter_EXPECT_correct_byte_sequence() {
  using namespace k2o;

  key<int(int, short, char), upd::endianess::LITTLE, upd::signed_mode::TWO_COMPLEMENT> k;
  auto dest_tuple = upd::make_tuple<int, short, char>(upd::little_endian, upd::two_complement);

  int i = 0;
  k(64, 32, 16) >> [&](byte_t byte) { *(dest_tuple.begin() + i++) = byte; };

  TEST_ASSERT_EQUAL_INT(64, dest_tuple.get<0>());
  TEST_ASSERT_EQUAL_INT(32, dest_tuple.get<1>());
  TEST_ASSERT_EQUAL_INT(16, dest_tuple.get<2>());
}

void key_DO_unserialize_data_sequence_EXPECT_correct_value() {
  using namespace k2o;

  key<int()> k;
  auto t = upd::make_tuple(int{64});

  int i = 0;
  auto value = k << [&]() { return t[i++]; };

  TEST_ASSERT_EQUAL_INT(t.get<0>(), value);
}

void key_DO_unserialize_data_sequence_with_parameter_EXPECT_correct_value() {
  using namespace k2o;

  key<int(int, short, char), upd::endianess::LITTLE, upd::signed_mode::TWO_COMPLEMENT> k;
  auto src_tuple = upd::make_tuple<>(upd::little_endian, upd::two_complement, int{64});

  int i = 0;
  auto result = k << [&]() { return *(src_tuple.begin() + i++); };

  TEST_ASSERT_EQUAL_INT(src_tuple.get<0>(), result);
}

void key_DO_create_key_from_ftor_signature_EXPECT_key_holding_ftor_signature() {
  using namespace k2o;

  auto ftor = [](int x) { return x; };
  key<decltype(ftor)> k;

  int i = 0, j = 0;
  auto buf = upd::make_tuple<int>();
  k(64) >> [&](byte_t byte) { buf[i++] = byte; };
  auto result = k << [&]() { return buf[j++]; };

  TEST_ASSERT_EQUAL_INT(64, result);
}

void key_DO_create_key_from_function_EXPECT_key_holding_function_signature_cpp17() {
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
