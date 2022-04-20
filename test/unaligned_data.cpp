#include <upd/unaligned_data.hpp>

#include "utility.hpp"

template<typename Int, Int V, upd::endianess Endianess, upd::signed_mode Signed_Mode, upd::byte_t... Expected_Bytes>
static void unaligned_data_DO_serialize_data_EXPECT_correct_raw_data() {
  constexpr upd::byte_t expected_data[] = {Expected_Bytes...};

  upd::unaligned_data<4 * sizeof(Int), Endianess, Signed_Mode> unaligned_data;
  unaligned_data.write_as(V, sizeof(Int));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, unaligned_data.begin() + sizeof(Int), sizeof(Int));
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
static void unaligned_data_DO_serialize_data_EXPECT_correct_value_when_unserializing() {
  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];

  upd::unaligned_data<4 * sizeof(int), Endianess, Signed_Mode> unaligned_data;
  unaligned_data.write_as(-0xabc, sizeof(int));

  snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(Endianess), static_cast<int>(Signed_Mode));
  TEST_ASSERT_EQUAL_HEX64_MESSAGE(-0xabc, unaligned_data.template read_as<int>(sizeof(int)), error_msg);
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
static void unaligned_data_DO_iterate_throught_content_EXPECT_correct_raw_data() {
  using namespace upd;

  uint8_t raw_data[]{0xaa, 0xbb, 0xcc};
  unaligned_data<sizeof(raw_data), Endianess, Signed_Mode> unaligned_data{raw_data};
  TEST_ASSERT_TRUE(unaligned_data.begin() != unaligned_data.end());
  std::size_t i = 0;
  for (auto byte : unaligned_data)
    TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

MAKE_MULTIOPT(unaligned_data_DO_serialize_data_EXPECT_correct_value_when_unserializing)
MAKE_MULTIOPT(unaligned_data_DO_iterate_throught_content_EXPECT_correct_raw_data)

int main() {
  using namespace upd;

  // Template instantiation check
  {
    byte_t raw_data[16]{};

    unaligned_data<16, endianess::LITTLE, signed_mode::TWO_COMPLEMENT>{};
    unaligned_data<16>{};

    make_unaligned_data<endianess::LITTLE, signed_mode::TWO_COMPLEMENT>(raw_data);
    make_unaligned_data(raw_data);
  }

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
                                                                     signed_mode::ONE_COMPLEMENT,
                                                                     0xbc,
                                                                     0x0a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -2,
                                                                     endianess::LITTLE,
                                                                     signed_mode::ONE_COMPLEMENT,
                                                                     0xfd,
                                                                     0xff>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::ONE_COMPLEMENT,
                                                                     0x0a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -2,
                                                                     endianess::BIG,
                                                                     signed_mode::ONE_COMPLEMENT,
                                                                     0xff,
                                                                     0xfd>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::LITTLE,
                                                                     signed_mode::TWO_COMPLEMENT,
                                                                     0xbc,
                                                                     0x0a>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -126,
                                                                     endianess::LITTLE,
                                                                     signed_mode::TWO_COMPLEMENT,
                                                                     0x82,
                                                                     0xff>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     0xabc,
                                                                     endianess::BIG,
                                                                     signed_mode::TWO_COMPLEMENT,
                                                                     0x0a,
                                                                     0xbc>));
  RUN_TEST((unaligned_data_DO_serialize_data_EXPECT_correct_raw_data<int16_t,
                                                                     -126,
                                                                     endianess::BIG,
                                                                     signed_mode::TWO_COMPLEMENT,
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
  unaligned_data_DO_iterate_throught_content_EXPECT_correct_raw_data_multiopt(every_options);
  return UNITY_END();
}
