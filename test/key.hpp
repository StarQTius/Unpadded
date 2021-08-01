#pragma once

#include <unity.h>

#include <k2o/key.hpp>
#include <k2o/type.hpp>

inline void key_DO_serialize_arguments_EXPECT_correct_byte_sequence() {
  using namespace k2o;

  key<int(int, const int(&)[8], char)> k;
  int array[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
  auto t = upd::make_tuple(int{64}, array, char{16});
  byte_t dest_buf[t.size];

  int i = 0;
  t.invoke(k) >> [&](byte_t byte) { dest_buf[i++] = byte; };

  TEST_ASSERT_EQUAL_UINT8_ARRAY(t.begin(), dest_buf, t.size);
}

inline void key_DO_unserialize_data_sequence_EXPECT_correct_value() {
  using namespace k2o;

  key<int()> k;
  auto t = upd::make_tuple(int{64});

  int i = 0;
  auto value = k << [&]() { return t[i++]; };

  TEST_ASSERT_EQUAL_INT(t.get<0>(), value);
}
