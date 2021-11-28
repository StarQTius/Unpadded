#include "tuple_view.hpp"

void run_tuple_view_ut() {
  using namespace upd;

  // Template instantiation check
  {
    constexpr size_t size = sizeof(short) + sizeof(int) + sizeof(long);

    byte_t buf[size];

    tuple_view<byte_t *, endianess::LITTLE, signed_mode::TWO_COMPLEMENT, int, char, bool>{buf};

    make_tuple_view<int, char, bool>(little_endian, two_complement, (byte_t *)buf);
    make_tuple_view<int, char, bool>((byte_t *)buf);
  }

  RUN_TEST(tuple_view_DO_bind_to_buffer_EXPECT_reading_correct_value);
  RUN_TEST(tuple_view_DO_set_value_EXPECT_reading_same_value);
  RUN_TEST(tuple_view_DO_bind_to_a_forward_list_EXPECT_correct_behavior);
}
