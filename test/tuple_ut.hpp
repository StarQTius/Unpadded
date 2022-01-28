#pragma once

#include <upd/tuple.hpp>

#include "utility.hpp"

namespace detail {

struct object_t {
  uint8_t x;
  uint16_t a, b;
};

#if __cplusplus >= 201703L

struct {
  template<typename View_T>
  static void serialize(const object_t &o, View_T &view) {
    upd::set<0>(view, o.x);
    upd::set<1>(view, o.a);
    upd::set<2>(view, o.b);
  }

  static object_t unserialize(uint8_t x, uint16_t a, uint16_t b) { return {x, a, b}; }
} extension;

inline auto upd_extension(object_t *) { return extension; }

#else // __cplusplus >= 201703L

struct object_extension_t {

  template<typename View_T>
  static void serialize(const object_t &o, View_T &view) {
    upd::set<0>(view, o.x);
    upd::set<1>(view, o.a);
    upd::set<2>(view, o.b);
  }

  static object_t unserialize(uint8_t x, uint16_t a, uint16_t b) { return {x, a, b}; }
};

inline object_extension_t upd_extension(object_t *) { return {}; }

#endif // __cplusplus >= 201703L

} // namespace detail

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
void tuple_DO_set_value_EXPECT_same_value_with_get() {
  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];

  upd::tuple<Endianess, Signed_Mode, short, int, long> tuple{0, 0, 0};
  tuple.template set<1>(-0xabc);

  snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(Endianess), static_cast<int>(Signed_Mode));
  TEST_ASSERT_EQUAL_HEX64_MESSAGE(-0xabc, tuple.template get<1>(), error_msg);
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
void tuple_DO_set_array_EXPECT_same_value_with_get() {
  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];

  int array[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
  upd::tuple<Endianess, Signed_Mode, decltype(array)> tuple;
  tuple.template set<0>(array);

  snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(Endianess), static_cast<int>(Signed_Mode));
  TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(array, tuple.template get<0>().content, sizeof array / sizeof(int), error_msg);
}

template<upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         upd::sfinae::require<Endianess == upd::endianess::BUILTIN> = 0>
void tuple_DO_iterate_throught_content_EXPECT_correct_raw_data() {
  using namespace upd;

  uint8_t raw_data[7]{};
  uint32_t data[]{0xaa, 0xbbcc, 0xddeeff00};

  memcpy(raw_data, data, 1);
  memcpy(raw_data + 1, data + 1, 2);
  memcpy(raw_data + 3, data + 2, 4);

  tuple<Endianess, Signed_Mode, uint8_t, uint16_t, uint32_t> tuple{0xaa, 0xbbcc, 0xddeeff00};
  TEST_ASSERT_TRUE(tuple.begin() != tuple.end());
  size_t i = 0;
  for (auto byte : tuple)
    TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

template<upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         upd::sfinae::require<Endianess == upd::endianess::LITTLE> = 0>
void tuple_DO_iterate_throught_content_EXPECT_correct_raw_data() {
  using namespace upd;

  uint8_t raw_data[]{0xaa, 0xcc, 0xbb, 0x00, 0xff, 0xee, 0xdd};
  tuple<Endianess, Signed_Mode, uint8_t, uint16_t, uint32_t> tuple{0xaa, 0xbbcc, 0xddeeff00};
  TEST_ASSERT_TRUE(tuple.begin() != tuple.end());
  size_t i = 0;
  for (auto byte : tuple)
    TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

template<upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         upd::sfinae::require<Endianess == upd::endianess::BIG> = 0>
void tuple_DO_iterate_throught_content_EXPECT_correct_raw_data() {
  using namespace upd;

  uint8_t raw_data[]{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00};
  tuple<Endianess, Signed_Mode, uint8_t, uint16_t, uint32_t> tuple{0xaa, 0xbbcc, 0xddeeff00};
  TEST_ASSERT_TRUE(tuple.begin() != tuple.end());
  size_t i = 0;
  for (auto byte : tuple)
    TEST_ASSERT_EQUAL_HEX16(raw_data[i++], byte);
}

template<upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         upd::sfinae::require<Endianess == upd::endianess::BUILTIN> = 0>
void tuple_DO_access_like_array_EXPECT_correct_raw_values() {
  using namespace upd;

  uint8_t raw_data[4]{};
  uint16_t data[]{0xaa, 0xbb, 0xccdd};

  memcpy(raw_data, data, 1);
  memcpy(raw_data + 1, data + 1, 1);
  memcpy(raw_data + 2, data + 2, 2);

  auto tuple = make_tuple(endianess_h<Endianess>{},
                          signed_mode_h<Signed_Mode>{},
                          (unsigned char){0xaa},
                          (unsigned char){0xbb},
                          (unsigned short){0xccdd});
  TEST_ASSERT_EQUAL_HEX8(raw_data[0], tuple.begin()[0]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[1], tuple.begin()[1]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[2], tuple.begin()[2]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[3], tuple.begin()[3]);
}

template<upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         upd::sfinae::require<Endianess == upd::endianess::LITTLE> = 0>
void tuple_DO_access_like_array_EXPECT_correct_raw_values() {
  using namespace upd;

  uint8_t raw_data[]{0xaa, 0xbb, 0xdd, 0xcc};
  auto tuple = make_tuple(endianess_h<Endianess>{},
                          signed_mode_h<Signed_Mode>{},
                          (unsigned char){0xaa},
                          (unsigned char){0xbb},
                          (unsigned short){0xccdd});
  TEST_ASSERT_EQUAL_HEX8(raw_data[0], tuple.begin()[0]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[1], tuple.begin()[1]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[2], tuple.begin()[2]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[3], tuple.begin()[3]);
}

template<upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         upd::sfinae::require<Endianess == upd::endianess::BIG> = 0>
void tuple_DO_access_like_array_EXPECT_correct_raw_values() {
  using namespace upd;

  uint8_t raw_data[]{0xaa, 0xbb, 0xcc, 0xdd};
  auto tuple = make_tuple(endianess_h<Endianess>{},
                          signed_mode_h<Signed_Mode>{},
                          (unsigned char){0xaa},
                          (unsigned char){0xbb},
                          (unsigned short){0xccdd});
  TEST_ASSERT_EQUAL_HEX8(raw_data[0], tuple.begin()[0]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[1], tuple.begin()[1]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[2], tuple.begin()[2]);
  TEST_ASSERT_EQUAL_HEX8(raw_data[3], tuple.begin()[3]);
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
void tuple_DO_invoke_function_EXPECT_correct_behavior() {
  using namespace upd;

  // Compile-time check
  {
    struct {
      void operator()(int) {}
    } f;
    struct {
      void operator()(int) const {}
    } cf;
    struct {
      void operator()(int) volatile {}
    } vf;
    struct {
      void operator()(int) const volatile {}
    } cvf;

    struct {
      void operator()(int) & {}
    } lf;
    struct {
      void operator()(int) const & {}
    } clf;
    struct {
      void operator()(int) volatile & {}
    } vlf;
    struct {
      void operator()(int) const volatile & {}
    } cvlf;

    struct rf_t {
      void operator()(int) && {}
    };
    struct crf_t {
      void operator()(int) const && {}
    };
    struct vrf_t {
      void operator()(int) volatile && {}
    };
    struct cvrf_t {
      void operator()(int) const volatile && {}
    };

    auto args = make_tuple(0);

    args.invoke(f);
    args.invoke(cf);
    args.invoke(vf);
    args.invoke(cvf);

    args.invoke(lf);
    args.invoke(clf);
    args.invoke(vlf);
    args.invoke(cvlf);

    args.invoke(rf_t{});
    args.invoke(crf_t{});
    args.invoke(vrf_t{});
    args.invoke(cvrf_t{});
  }

  auto terms = make_tuple(endianess_h<Endianess>{}, signed_mode_h<Signed_Mode>{}, int{12}, int{34}, int{-56});

  TEST_ASSERT_EQUAL_INT(12 + 34 - 56, terms.invoke(*(+[](int a, int b, int c) { return a + b + c; })));
}

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
void tuple_DO_make_empty_tuple_EXPECT_valid_object() {
  using namespace upd;

  const char error_format[] = "endianess = %i, signed mode = %i";
  char error_msg[sizeof(error_format)];
  snprintf(error_msg, sizeof(error_msg), error_format, static_cast<int>(Endianess), static_cast<int>(Signed_Mode));

  auto empty_tuple = make_tuple(endianess_h<Endianess>{}, signed_mode_h<Signed_Mode>{});

  TEST_ASSERT_EQUAL_INT_MESSAGE(0, empty_tuple.size, error_msg);
}

inline void tuple_DO_convert_to_array_EXPECT_same_content() {
  using namespace upd;

  {
    array_wrapper<int[16]> awrapper;
    int(&a)[16] = awrapper;

    TEST_ASSERT_EQUAL_INT_ARRAY(awrapper.content, a, sizeof a / sizeof *a);
  }
}

inline void tuple_DO_construct_tuple_with_ctad_EXPECT_tuple_holds_correct_values_cpp17() {
#if __cplusplus >= 201703L
  using namespace upd;

  tuple t1{little_endian, two_complement, int{64}, short{16}, char{8}};
  tuple t2{int{64}, short{16}, char{8}};

  TEST_ASSERT_EQUAL_INT(t1.get<0>(), t2.get<0>());
  TEST_ASSERT_EQUAL_INT(t1.get<1>(), t2.get<1>());
  TEST_ASSERT_EQUAL_INT(t1.get<2>(), t2.get<2>());
#endif // __cplusplus >= 201703L
}

inline void tuple_DO_bind_names_to_tuple_element_EXPECT_getting_same_values_cpp17() {
#if __cplusplus >= 201703L
  using namespace upd;

  int array[]{0x00, 0x11, 0x22, 0x33};
  tuple t{int{64}, array, short{16}, char{8}};

  auto [a, b, c, d] = t;
  TEST_ASSERT_EQUAL_INT(64, a);
  TEST_ASSERT_EQUAL_INT_ARRAY(array, b, sizeof array / sizeof *array);
  TEST_ASSERT_EQUAL_INT(16, c);
  TEST_ASSERT_EQUAL_INT(8, d);
#endif // __cplusplus >= 201703L
}

inline void tuple_DO_serialize_user_provided_structure_EXCEPT_correct_behavior() {
  using namespace upd;

  ::detail::object_t o{0xa, 0xb, 0xc};

  auto t = make_tuple(int32_t{0xd}, o, int16_t{0xe});
  TEST_ASSERT_EQUAL_INT32(t.get<0>(), 0xd);
  TEST_ASSERT_EQUAL_UINT8(t.get<1>().x, 0xa);
  TEST_ASSERT_EQUAL_UINT16(t.get<1>().a, 0xb);
  TEST_ASSERT_EQUAL_UINT16(t.get<1>().b, 0xc);
  TEST_ASSERT_EQUAL_INT16(t.get<2>(), 0xe);
  TEST_ASSERT_EQUAL_INT16(t.size, 4 + 5 + 2);
}

inline void tuple_view_DO_iterate_subview_EXPECT_exact_subsequence() {
  using namespace upd;

  auto t = make_tuple(int{64}, char{16}, int{32}, bool{true});
  byte_t *s_seq = t.begin(), *m_seq = t.begin() + sizeof(int), *e_seq = t.begin() + sizeof(int) + sizeof(char);
  for (auto byte : t.view<0, 2>())
    TEST_ASSERT_EQUAL_UINT8(*s_seq++, byte);
  for (auto byte : t.view<1, 2>())
    TEST_ASSERT_EQUAL_UINT8(*m_seq++, byte);
  for (auto byte : t.view<2, 2>())
    TEST_ASSERT_EQUAL_UINT8(*e_seq++, byte);
}

MAKE_MULTIOPT(tuple_DO_set_value_EXPECT_same_value_with_get)
MAKE_MULTIOPT(tuple_DO_set_array_EXPECT_same_value_with_get)
MAKE_MULTIOPT(tuple_DO_iterate_throught_content_EXPECT_correct_raw_data)
MAKE_MULTIOPT(tuple_DO_access_like_array_EXPECT_correct_raw_values)
MAKE_MULTIOPT(tuple_DO_invoke_function_EXPECT_correct_behavior)
MAKE_MULTIOPT(tuple_DO_make_empty_tuple_EXPECT_valid_object)
