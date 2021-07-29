#include "main.hpp"

void setup() {
  UNITY_BEGIN();

  RUN_TEST(order_DO_serialize_argument_into_stream_EXPECT_order_getting_unaltered_argument);
  RUN_TEST(order_DO_give_then_return_argument_from_order_EXPECT_unaltered_value);
  RUN_TEST(order_DO_give_then_return_array_from_order_EXPECT_unaltered_value);

  UNITY_END();
}

void loop() {}
