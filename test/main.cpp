#include "main.hpp"

void setup() {
  UNITY_BEGIN();

  RUN_TEST(order_DO_serialize_argument_into_stream_EXPECT_order_getting_unaltered_argument);

  UNITY_END();
}

void loop() {}
