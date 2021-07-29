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

inline void order_DO_give_then_return_argument_from_order_EXPECT_unaltered_value() {
  using namespace k2o;

  constexpr int argument = 0xabc;
  auto serialized_argument = upd::make_tuple(argument);
  auto serialized_return_value = upd::make_tuple(int{0});
  order return_argument{[](int value) { return value; }};

  size_t i = 0, j = 0;
  auto error = return_argument([&]() { return serialized_argument[i++]; },
                               [&](byte_t byte) { serialized_return_value[j++] = byte; });

  TEST_ASSERT_EQUAL_UINT(status::OK, error);
  TEST_ASSERT_EQUAL_INT(argument, serialized_return_value.get<0>());
}

inline void order_DO_give_then_return_array_from_order_EXPECT_unaltered_value() {
  using namespace k2o;

  constexpr int argument[] = {
      0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
  constexpr auto argument_size = sizeof argument / sizeof *argument;
  auto serialized_argument = upd::make_tuple(argument);
  decltype(serialized_argument) serialized_return_value;
  order return_array{[](const int(&array)[argument_size]) { return upd::make_tuple(array); }};

  size_t i = 0, j = 0;
  auto error = return_array([&]() { return serialized_argument[i++]; },
                            [&](byte_t byte) { serialized_return_value[j++] = byte; });

  TEST_ASSERT_EQUAL_UINT(status::OK, error);
  TEST_ASSERT_EQUAL_INT_ARRAY(argument, serialized_return_value.get<0>().content, argument_size);
}
