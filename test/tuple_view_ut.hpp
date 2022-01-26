#pragma once

#include <upd/tuple.hpp>

inline void tuple_view_DO_bind_to_buffer_EXPECT_reading_correct_value() {
  using namespace upd;

  constexpr size_t size = sizeof(short) + sizeof(int) + sizeof(long);

  byte_t buf[size];
  auto tview = make_view<short, int, long>(builtin_endianess, builtin_signed_mode, (byte_t *)buf);

  write_as<endianess::BUILTIN, signed_mode::BUILTIN>(int{-0xabc}, buf + sizeof(short));

  TEST_ASSERT_EQUAL_HEX64(-0xabc, tview.get<1>());
}

inline void tuple_view_DO_set_value_EXPECT_reading_same_value() {
  using namespace upd;

  constexpr size_t size = sizeof(short) + sizeof(int) + sizeof(long);

  byte_t buf[size];
  auto tview = make_view<short, int, long>(builtin_endianess, builtin_signed_mode, (byte_t *)buf);

  tview.set<1>(-0xabc);

  TEST_ASSERT_EQUAL_HEX64(-0xabc, tview.get<1>());
}

inline void tuple_view_DO_bind_to_a_forward_list_EXPECT_correct_behavior() {
  using namespace upd;

  struct node {
    node() : next{nullptr} {}
    node(node *next) : next{next} {}
    ~node() { delete next; }

    byte_t value;
    node *next;
  };

  struct iterator {
    iterator(node *ptr) : ptr{ptr} {}

    iterator operator++(int) {
      auto it = *this;
      ptr = ptr->next;
      return it;
    }
    byte_t &operator*() { return ptr->value; }

    node *ptr;
  };

  node root{new node{new node{new node{new node{}}}}};
  iterator begin{&root};

  auto tview = make_view<uint8_t, uint32_t>(builtin_endianess, builtin_signed_mode, begin);

  tview.set<1>(0xabc);

  TEST_ASSERT_EQUAL_HEX8(0xabc, tview.get<1>());
}

inline void tuple_view_DO_assign_to_a_tuple_EXPECT_correct_behavior() {
  using namespace upd;

  auto lhs = make_tuple(int{}, char{}, bool{}), rhs = make_tuple(int{0xa}, char{0xb}, bool{true});
  auto result = (lhs = make_view<int, char, bool>(rhs.begin()));

  static_assert(boost::is_same<decltype(result), decltype(lhs)>::value);
  TEST_ASSERT_EQUAL_INT(get<0>(lhs), 0xa);
  TEST_ASSERT_EQUAL_CHAR(get<1>(lhs), 0xb);
  TEST_ASSERT_TRUE(get<2>(lhs));
}
