#include <unity.h>

#include "tuple_view.hpp"

int main() {
  using namespace upd;

  // Template instantiation check
  {
    constexpr std::size_t size = sizeof(short) + sizeof(int) + sizeof(long);

    byte_t buf[size];

    tuple_view<byte_t *, endianess::LITTLE, signed_mode::TWO_COMPLEMENT, int, char, bool>{buf};

    make_view<int, char, bool>(little_endian, two_complement, (byte_t *)buf);
    make_view<int, char, bool>((byte_t *)buf);
  }

  UNITY_BEGIN();
  RUN_TEST(tuple_view_DO_bind_to_buffer_EXPECT_reading_correct_value);
  RUN_TEST(tuple_view_DO_set_value_EXPECT_reading_same_value);
  RUN_TEST(tuple_view_DO_bind_to_a_forward_list_EXPECT_correct_behavior);
  RUN_TEST(tuple_view_DO_assign_to_a_tuple_EXPECT_correct_behavior);
  return UNITY_END();
}
