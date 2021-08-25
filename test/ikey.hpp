#pragma once

#include <k2o/ikey.hpp>
#include <k2o/type.hpp>

inline void ikey_DO_serialize_argument_EXPECT_correct_id_and_result() {
  k2o::ikey<16, int(int)> ikey;

  auto buf = upd::make_tuple(size_t{0}, int{0});
  size_t i = 0;
  ikey(64) >> [&](k2o::byte_t byte) { buf[i++] = byte; };

  TEST_ASSERT_EQUAL_INT(16, buf.get<0>());
  TEST_ASSERT_EQUAL_INT(64, buf.get<1>());
}
