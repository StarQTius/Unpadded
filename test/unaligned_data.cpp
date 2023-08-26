#include "utility.hpp"
#include <upd/detail/serialization.hpp>

template<typename Int, Int V, upd::endianess Endianess, upd::signed_mode Signed_Mode, upd::byte_t... Expected_Bytes>
static void unaligned_data_DO_serialize_data_EXPECT_correct_raw_data() {
  using namespace upd;

  byte_t expected_data[] = {Expected_Bytes...}, buf[4 * sizeof(Int)];

  detail::write_as<Endianess, Signed_Mode>(V, buf + sizeof(Int));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, buf + sizeof(Int), sizeof(Int));
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
static void unaligned_data_DO_serialize_data_EXPECT_correct_value_when_unserializing() {
  using namespace upd;

  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];
  byte_t buf[4 * sizeof(int)];

  detail::write_as<Endianess, Signed_Mode>(-0xabc, buf + sizeof(int));

  snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(Endianess), static_cast<int>(Signed_Mode));
  TEST_ASSERT_EQUAL_HEX64_MESSAGE(-0xabc, (detail::read_as<int, Endianess, Signed_Mode>(buf + sizeof(int))), error_msg);
}

MAKE_MULTIOPT(unaligned_data_DO_serialize_data_EXPECT_correct_value_when_unserializing)

int main() {
  using namespace upd;

  // Check every combination of endianess and signed representation
  UNITY_BEGIN();
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::SIGNED_MAGNITUDE,
                                                                     0xbc,
                                                                     0x0a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::SIGNED_MAGNITUDE,
                                                                     0xbc,
                                                                     0x8a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::SIGNED_MAGNITUDE,
                                                                     0x0a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::SIGNED_MAGNITUDE,
                                                                     0x8a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::ONES_COMPLEMENT,
                                                                     0xbc,
                                                                     0x0a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -2,
                                                                     endianess::LITTLE,
                                                                     signed_mode::ONES_COMPLEMENT,
                                                                     0xfd,
                                                                     0xff>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::ONES_COMPLEMENT,
                                                                     0x0a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -2,
                                                                     endianess::BIG,
                                                                     signed_mode::ONES_COMPLEMENT,
                                                                     0xff,
                                                                     0xfd>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::TWOS_COMPLEMENT,
                                                                     0xbc,
                                                                     0x0a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -126,
                                                                     endianess::LITTLE,
                                                                     signed_mode::TWOS_COMPLEMENT,
                                                                     0x82,
                                                                     0xff>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::TWOS_COMPLEMENT,
                                                                     0x0a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -126,
                                                                     endianess::BIG,
                                                                     signed_mode::TWOS_COMPLEMENT,
                                                                     0xff,
                                                                     0x82>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::OFFSET_BINARY,
                                                                     0xbc,
                                                                     0x8a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::OFFSET_BINARY,
                                                                     0x44,
                                                                     0x75>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::OFFSET_BINARY,
                                                                     0x8a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::OFFSET_BINARY,
                                                                     0x75,
                                                                     0x44>));

  unaligned_data_DO_serialize_data_EXPECT_correct_value_when_unserializing_multiopt(every_options);
  return UNITY_END();
}
