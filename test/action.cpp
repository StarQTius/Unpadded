#include <upd/action.hpp>

#include "utility.hpp"

static void action_DO_serialize_argument_into_stream_EXPECT_action_getting_unaltered_argument() {
  using namespace upd;

  uint8_t value;
  action assign_to_value{[&](uint8_t x) { value = x; }};

  assign_to_value([]() -> upd::byte_t { return 0xaf; });

  TEST_ASSERT_EQUAL_INT(0xaf, value);
}

static void action_DO_give_then_return_argument_from_action_EXPECT_unaltered_value() {
  using namespace upd;

  constexpr int argument = 0xabc;
  auto serialized_argument = upd::make_tuple(argument);
  auto serialized_return_value = upd::make_tuple(int{0});
  action return_argument{[](int value) { return value; }};

  size_t i = 0, j = 0;
  return_argument([&]() { return serialized_argument[i++]; },
                  [&](upd::byte_t byte) { serialized_return_value[j++] = byte; });

  TEST_ASSERT_EQUAL_INT(argument, serialized_return_value.get<0>());
}

static void action_DO_instantiate_action_with_functor_taking_arguments_EXPECT_input_and_output_sizes_correct() {
  using namespace upd;

  action f([](int, int(&)[16], char) -> int { return 0; });

  TEST_ASSERT_EQUAL_INT(sizeof(int) + 16 * sizeof(int) + sizeof(char), f.input_size());
  TEST_ASSERT_EQUAL_INT(sizeof(int), f.output_size());
}

static void action_DO_instantiate_action_with_functor_taking_no_arguments_EXPECT_input_and_output_sizes_correct() {
  using namespace upd;

  action f([]() -> int { return 0; });

  TEST_ASSERT_EQUAL_INT(0, f.input_size());
  TEST_ASSERT_EQUAL_INT(sizeof(int), f.output_size());
}

static void action_DO_instantiate_action_with_functor_returning_non_tuple_EXPECT_input_and_output_sizes_correct() {
  using namespace upd;

  action f([](int) -> int { return 0; });

  TEST_ASSERT_EQUAL_INT(sizeof(int), f.input_size());
  TEST_ASSERT_EQUAL_INT(sizeof(int), f.output_size());
}

static void action_DO_instantiate_action_with_functor_non_returning_EXPECT_input_and_output_sizes_correct() {
  using namespace upd;

  action f([](int) {});

  TEST_ASSERT_EQUAL_INT(sizeof(int), f.input_size());
  TEST_ASSERT_EQUAL_INT(0, f.output_size());
}

#define ACTION DECLVAL(action)

int main() {
  using namespace upd;

  DETECT(
      ACTION(READABLE, WRITABLE), ACTION(READABLE, BYTE_PTR), ACTION(BYTE_PTR, WRITABLE), ACTION(BYTE_PTR, BYTE_PTR));

  UNITY_BEGIN();
  RUN_TEST(action_DO_serialize_argument_into_stream_EXPECT_action_getting_unaltered_argument);
  RUN_TEST(action_DO_give_then_return_argument_from_action_EXPECT_unaltered_value);
  RUN_TEST(action_DO_instantiate_action_with_functor_taking_arguments_EXPECT_input_and_output_sizes_correct);
  RUN_TEST(action_DO_instantiate_action_with_functor_taking_no_arguments_EXPECT_input_and_output_sizes_correct);
  RUN_TEST(action_DO_instantiate_action_with_functor_returning_non_tuple_EXPECT_input_and_output_sizes_correct);
  RUN_TEST(action_DO_instantiate_action_with_functor_non_returning_EXPECT_input_and_output_sizes_correct);
  return UNITY_END();
}
