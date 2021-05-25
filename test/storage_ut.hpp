#pragma once

#include <cstring>

#include "upd/storage.hpp"
#include "unity.h"

template<upd::endianess Endianess>
struct endianess_h {};

template<upd::signed_mode Signed_Mode>
struct signed_mode_h {};

template<typename, typename>
struct options_h;

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct options_h<endianess_h<Endianess>, signed_mode_h<Signed_Mode>> {};

#define MAKE_MULTIOPT(FUNCTION_NAME) \
  template<upd::endianess... Endianesses, upd::signed_mode... Signed_Modes> \
  void FUNCTION_NAME ## _multiopt( \
    boost::mp11::mp_list<options_h<endianess_h<Endianesses>, signed_mode_h<Signed_Modes>>...>) \
  { \
    using discard = int[]; \
    discard {(RUN_TEST((FUNCTION_NAME<Endianesses, Signed_Modes>)), 0)...}; \
  }

template<typename Int, Int V, upd::endianess Endianess, upd::signed_mode Signed_Mode, upd::byte_t... Expected_Bytes>
void storage_write_and_inspect_raw_data() {
  constexpr upd::byte_t expected_data[] = {Expected_Bytes...};

  upd::unaligned_data<4 * sizeof(Int), Endianess, Signed_Mode> unaligned_data;
  unaligned_data.write(V, sizeof(Int));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_data, unaligned_data.begin() + sizeof(Int), sizeof(Int));
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
void storage_write_and_interpret() {
  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];

  upd::unaligned_data<4 * sizeof(int), Endianess, Signed_Mode> unaligned_data;
  unaligned_data.write(-0xabc, sizeof(int));

  snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(Endianess), static_cast<int>(Signed_Mode));
  TEST_ASSERT_EQUAL_HEX64_MESSAGE(-0xabc, unaligned_data.template interpret_as<int>(sizeof(int)), error_msg);
}

MAKE_MULTIOPT(storage_write_and_interpret)

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
void storage_set_and_get() {
  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];

  upd::tuple<Endianess, Signed_Mode, short, int, long> tuple{0, 0, 0};
  tuple.template set<1>(-0xabc);

  snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(Endianess), static_cast<int>(Signed_Mode));
  TEST_ASSERT_EQUAL_HEX64_MESSAGE(-0xabc, tuple.template get<1>(), error_msg);
}

MAKE_MULTIOPT(storage_set_and_get)

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
void storage_iterate_unaligned_data() {
  using namespace upd;

  uint8_t raw_data[] {0xaa, 0xbb, 0xcc};
  unaligned_data<sizeof(raw_data), Endianess, Signed_Mode> unaligned_data{raw_data};
  TEST_ASSERT_TRUE(unaligned_data.begin() != unaligned_data.end());
  size_t i = 0;
  for (auto byte : unaligned_data) TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

MAKE_MULTIOPT(storage_iterate_unaligned_data)

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
upd::sfinae::enable_t<Endianess == upd::endianess::BUILTIN>
storage_iterate_tuple() {
  using namespace upd;

  uint8_t raw_data[7] {};
  uint32_t data[] {0xaa, 0xbbcc, 0xddeeff00};

  memcpy(raw_data, data, 1);
  memcpy(raw_data + 1, data + 1, 2);
  memcpy(raw_data + 3, data + 2, 4);

  tuple<Endianess, Signed_Mode, uint8_t, uint16_t, uint32_t> tuple{
    0xaa,
    0xbbcc,
    0xddeeff00};
  TEST_ASSERT_TRUE(tuple.begin() != tuple.end());
  size_t i = 0;
  for (auto byte : tuple) TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
upd::sfinae::enable_t<Endianess == upd::endianess::LITTLE>
storage_iterate_tuple() {
  using namespace upd;

  uint8_t raw_data[] {0xaa, 0xcc, 0xbb, 0x00, 0xff, 0xee, 0xdd};
  tuple<Endianess, Signed_Mode, uint8_t, uint16_t, uint32_t> tuple{
    0xaa,
    0xbbcc,
    0xddeeff00};
  TEST_ASSERT_TRUE(tuple.begin() != tuple.end());
  size_t i = 0;
  for (auto byte : tuple) TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
upd::sfinae::enable_t<Endianess == upd::endianess::BIG>
storage_iterate_tuple() {
  using namespace upd;

  uint8_t raw_data[] {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
  tuple<Endianess, Signed_Mode, uint8_t, uint16_t, uint32_t> tuple{
    0xaa,
    0xbbcc,
    0xddeeff00};
  TEST_ASSERT_TRUE(tuple.begin() != tuple.end());
  size_t i = 0;
  for (auto byte : tuple) TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

MAKE_MULTIOPT(storage_iterate_tuple)

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
upd::sfinae::enable_t<Endianess == upd::endianess::BUILTIN>
storage_access_raw_data() {
  using namespace upd;

  uint8_t raw_data[4] {};
  uint16_t data[] {0xaa, 0xbb, 0xccdd};

  memcpy(raw_data, data, 1);
  memcpy(raw_data + 1, data + 1, 1);
  memcpy(raw_data + 2, data + 2, 2);

  auto tuple = make_tuple<Endianess, Signed_Mode>(
    (unsigned char){0xaa},
    (unsigned char){0xbb},
    (unsigned short){0xccdd});
  TEST_ASSERT_EQUAL_HEX8(raw_data[0], tuple.begin()[0]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[1], tuple.begin()[1]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[2], tuple.begin()[2]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[3], tuple.begin()[3]);
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
upd::sfinae::enable_t<Endianess == upd::endianess::LITTLE>
storage_access_raw_data() {
  using namespace upd;

  uint8_t raw_data[] {0xaa, 0xbb, 0xdd, 0xcc};
  auto tuple = make_tuple<Endianess, Signed_Mode>(
    (unsigned char){0xaa},
    (unsigned char){0xbb},
    (unsigned short){0xccdd});
  TEST_ASSERT_EQUAL_HEX8(raw_data[0], tuple.begin()[0]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[1], tuple.begin()[1]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[2], tuple.begin()[2]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[3], tuple.begin()[3]);
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
upd::sfinae::enable_t<Endianess == upd::endianess::BIG>
storage_access_raw_data() {
  using namespace upd;

  uint8_t raw_data[] {0xaa, 0xbb, 0xcc, 0xdd};
  auto tuple = make_tuple<Endianess, Signed_Mode>(
    (unsigned char){0xaa},
    (unsigned char){0xbb},
    (unsigned short){0xccdd});
  TEST_ASSERT_EQUAL_HEX8(raw_data[0], tuple.begin()[0]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[1], tuple.begin()[1]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[2], tuple.begin()[2]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[3], tuple.begin()[3]);
}

MAKE_MULTIOPT(storage_access_raw_data)
