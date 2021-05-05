#pragma once

#include <cstring>

#include "upd/storage.hpp"
#include "unity.h"

template<typename Int, Int V, upd::endianess endianess, upd::signed_mode signed_mode, upd::byte_t... Expected_Bytes>
inline void storage_write_and_inspect_raw_data() {
  constexpr upd::byte_t expected_data[] = {Expected_Bytes...};

  upd::unaligned_data<4 * sizeof(Int)> unaligned_data{endianess, signed_mode};
  unaligned_data.write(V, sizeof(Int));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, unaligned_data.begin() + sizeof(Int), sizeof(Int));
}

template<typename Int, Int V>
inline void storage_write_and_interpret() {
  constexpr upd::endianess endianesses[] = { upd::endianess::LITTLE, upd::endianess::BIG };
  constexpr upd::signed_mode signed_modes[] = {
    upd::signed_mode::ONE_COMPLEMENT,
    upd::signed_mode::TWO_COMPLEMENT
  };

  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];

  for (auto endianess : endianesses) {
    for (auto signed_mode : signed_modes) {
      upd::unaligned_data<4 * sizeof(Int)> unaligned_data{endianess, signed_mode};
      unaligned_data.write(V, sizeof(Int));

      snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(endianess), static_cast<int>(signed_mode));
      TEST_ASSERT_EQUAL_HEX64_MESSAGE(V, unaligned_data.template interpret_as<Int>(sizeof(Int)), error_msg);
    }
  }
}

template<size_t I, long long V, typename... Args>
inline void storage_set_and_get() {
  constexpr upd::endianess endianesses[] = { upd::endianess::LITTLE, upd::endianess::BIG };
  constexpr upd::signed_mode signed_modes[] = {
    upd::signed_mode::ONE_COMPLEMENT,
    upd::signed_mode::TWO_COMPLEMENT
  };

  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];

  for (auto endianess : endianesses) {
    for (auto signed_mode : signed_modes) {
      upd::unaligned_tuple<Args...> unaligned_tuple{endianess, signed_mode};
      unaligned_tuple.template set<I>(V);

      snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(endianess), static_cast<int>(signed_mode));
      TEST_ASSERT_EQUAL_HEX64_MESSAGE(V, unaligned_tuple.template get<I>(), error_msg);
    }
  }
}

inline void storage_iterate_unaligned_data() {
  using namespace upd;

  uint8_t raw_data[] {0xaa, 0xbb, 0xcc};
  unaligned_data<sizeof(raw_data)> unaligned_data{raw_data, endianess::LITTLE, signed_mode::ONE_COMPLEMENT};
  TEST_ASSERT_TRUE(unaligned_data.begin() != unaligned_data.end());
  size_t i = 0;
  for (auto byte : unaligned_data) TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

inline void storage_iterate_unaligned_arguments() {
  using namespace upd;

  uint8_t raw_data[] {0xaa, 0xcc, 0xbb, 0x00, 0xff, 0xee, 0xdd};
  unaligned_tuple<uint8_t, uint16_t, uint32_t> unaligned_arguments{
    endianess::LITTLE, signed_mode::ONE_COMPLEMENT,
    0xaa,
    0xbbcc,
    0xddeeff00};
  TEST_ASSERT_TRUE(unaligned_arguments.begin() != unaligned_arguments.end());
  size_t i = 0;
  for (auto byte : unaligned_arguments) TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

inline void storage_access_raw_data() {
  using namespace upd;

  uint8_t raw_data[] {0xaa, 0xbb, 0xcc, 0xdd};
  auto unaligned_arguments = make_unaligned_arguments(
    endianess::BIG, signed_mode::ONE_COMPLEMENT,
    uint8_t{0xaa},
    uint8_t{0xbb},
    uint16_t{0xccdd});
  TEST_ASSERT_EQUAL_HEX8(raw_data[0], unaligned_arguments.begin()[0]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[1], unaligned_arguments.begin()[1]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[2], unaligned_arguments.begin()[2]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[3], unaligned_arguments.begin()[3]);
}
