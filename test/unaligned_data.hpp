#pragma once

#include <upd/unaligned_data.hpp>

#include "utility.hpp"

template<typename Int, Int V, upd::endianess Endianess, upd::signed_mode Signed_Mode, upd::byte_t... Expected_Bytes>
void unaligned_data_DO_serialize_data_EXPECT_correct_raw_data() {
  constexpr upd::byte_t expected_data[] = {Expected_Bytes...};

  upd::unaligned_data<4 * sizeof(Int), Endianess, Signed_Mode> unaligned_data;
  unaligned_data.write_as(V, sizeof(Int));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, unaligned_data.begin() + sizeof(Int), sizeof(Int));
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
void unaligned_data_DO_serialize_data_EXPECT_correct_value_when_unserializing() {
  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];

  upd::unaligned_data<4 * sizeof(int), Endianess, Signed_Mode> unaligned_data;
  unaligned_data.write_as(-0xabc, sizeof(int));

  snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(Endianess), static_cast<int>(Signed_Mode));
  TEST_ASSERT_EQUAL_HEX64_MESSAGE(-0xabc, unaligned_data.template read_as<int>(sizeof(int)), error_msg);
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
void unaligned_data_DO_iterate_throught_content_EXPECT_correct_raw_data() {
  using namespace upd;

  uint8_t raw_data[]{0xaa, 0xbb, 0xcc};
  unaligned_data<sizeof(raw_data), Endianess, Signed_Mode> unaligned_data{raw_data};
  TEST_ASSERT_TRUE(unaligned_data.begin() != unaligned_data.end());
  size_t i = 0;
  for (auto byte : unaligned_data)
    TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

enum senum_t : int16_t { SV = -128 };
template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
inline void unaligned_data_DO_serialize_unsigned_enum_EXPECT_behave_like_operating_on_underlying_type() {
  using namespace upd;

  senum_t e{SV};

  unaligned_data<sizeof e, Endianess, Signed_Mode> udata;
  write_as(e, udata, 0);

  TEST_ASSERT_EQUAL_INT(e, read_as<decltype(e)>(udata, 0));
}

enum uenum_t : uint16_t { UV = 255 };
template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
inline void unaligned_data_DO_serialize_signed_enum_EXPECT_behave_like_operating_on_underlying_type() {
  using namespace upd;

  uenum_t e{UV};

  unaligned_data<sizeof e, Endianess, Signed_Mode> udata;
  write_as(e, udata, 0);

  TEST_ASSERT_EQUAL_UINT(e, read_as<decltype(e)>(udata, 0));
}

MAKE_MULTIOPT(unaligned_data_DO_serialize_data_EXPECT_correct_value_when_unserializing)
MAKE_MULTIOPT(unaligned_data_DO_iterate_throught_content_EXPECT_correct_raw_data)
MAKE_MULTIOPT(unaligned_data_DO_serialize_unsigned_enum_EXPECT_behave_like_operating_on_underlying_type)
MAKE_MULTIOPT(unaligned_data_DO_serialize_signed_enum_EXPECT_behave_like_operating_on_underlying_type)
