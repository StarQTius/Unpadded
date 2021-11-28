#pragma once

#include <upd/tuple.hpp>

inline void tuple_view_DO_bind_to_buffer_EXPECT_reading_correct_value() {
  using namespace upd;

  constexpr size_t size = sizeof(short) + sizeof(int) + sizeof(long);

  byte_t buf[size];
  auto tview = make_tuple_view<short, int, long>(builtin_endianess, builtin_signed_mode, (byte_t *)buf);

  write_as<endianess::BUILTIN, signed_mode::BUILTIN>(int{-0xabc}, buf + sizeof(short));

  TEST_ASSERT_EQUAL_HEX64(-0xabc, tview.get<1>());
}

inline void tuple_view_DO_set_value_EXPECT_reading_same_value() {
  using namespace upd;

  constexpr size_t size = sizeof(short) + sizeof(int) + sizeof(long);

  byte_t buf[size];
  auto tview = make_tuple_view<short, int, long>(builtin_endianess, builtin_signed_mode, (byte_t *)buf);

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

  auto tview = make_tuple_view<uint8_t, uint32_t>(builtin_endianess, builtin_signed_mode, begin);

  tview.set<1>(0xabc);

  TEST_ASSERT_EQUAL_HEX8(0xabc, tview.get<1>());
}
