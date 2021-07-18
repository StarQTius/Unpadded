#pragma once

#include <unity.h>

#include <k2o/order.hpp>
#include <k2o/status.hpp>

inline void order_DO_serialize_argument_into_stream_EXPECT_order_getting_unaltered_argument() {
  using namespace k2o;

  uint8_t value;
  order assign_to_value{[&](uint8_t x) { value = x; }};

  auto error = assign_to_value([]() -> byte_t { return 0xaf; });

  TEST_ASSERT_EQUAL_UINT(status::OK, error);
  TEST_ASSERT_EQUAL_INT(0xaf, value);
}
