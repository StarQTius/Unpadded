#include <upd/tuple.hpp>

#include "utility.hpp"

static void tuple_view_DO_bind_to_buffer_EXPECT_reading_correct_value() {
  using namespace upd;

  constexpr std::size_t size = sizeof(short) + sizeof(int) + sizeof(long);

  byte_t buf[size];
  auto tview = make_view<short, int, long>(builtin_endianess, builtin_signed_mode, (byte_t *)buf);

  write_as<endianess::BUILTIN, signed_mode::BUILTIN>(int{-0xabc}, buf + sizeof(short));

  TEST_ASSERT_EQUAL_HEX64(-0xabc, tview.get<1>());
}

static void tuple_view_DO_set_value_EXPECT_reading_same_value() {
  using namespace upd;

  constexpr std::size_t size = sizeof(short) + sizeof(int) + sizeof(long);

  byte_t buf[size];
  auto tview = make_view<short, int, long>(builtin_endianess, builtin_signed_mode, (byte_t *)buf);

  tview.set<1>(-0xabc);

  TEST_ASSERT_EQUAL_HEX64(-0xabc, tview.get<1>());
}

static void tuple_view_DO_bind_to_a_forward_list_EXPECT_correct_behavior() {
  using namespace upd;

  struct node {
    node() : next{nullptr} {}
    node(node *next) : next{next} {}
    ~node() { delete next; }

    byte_t value;
    node *next;
  };

  struct iterator {
    using difference_type = std::size_t;
    using value_type = byte_t;
    using pointer = byte_t *;
    using reference = byte_t &;
    using iterator_category = std::forward_iterator_tag;

    iterator(node *ptr) : ptr{ptr} {}

    iterator operator++() {
      ptr = ptr->next;
      return *this;
    }

    iterator operator++(int) {
      auto cpy = *this;
      ptr = ptr->next;
      return cpy;
    }

    byte_t &operator*() { return ptr->value; }
    byte_t *operator->() { return &(ptr->value); }

    node *ptr;
  };

  // GCC complains if local typedefs are not used
  [[maybe_unused]] iterator::difference_type a;
  [[maybe_unused]] iterator::value_type b;
  [[maybe_unused]] iterator::pointer c;
  [[maybe_unused]] iterator::reference d = b;
  [[maybe_unused]] iterator::iterator_category e;

  node root{new node{new node{new node{new node{}}}}};
  iterator begin{&root};

  auto tview = make_view<uint8_t, uint32_t>(builtin_endianess, builtin_signed_mode, begin);

  tview.set<1>(0xabc);

  TEST_ASSERT_EQUAL_HEX8(0xabc, tview.get<1>());
}

static void tuple_view_DO_assign_to_a_tuple_EXPECT_correct_behavior() {
  using namespace upd;

  auto lhs = make_tuple(int{}, char{}, bool{}), rhs = make_tuple(int{0xa}, char{0xb}, bool{true});
  auto result = (lhs = make_view<int, char, bool>(rhs.begin()));

  static_assert(std::is_same<decltype(result), decltype(lhs)>::value);
  TEST_ASSERT_EQUAL_INT(get<0>(lhs), 0xa);
  TEST_ASSERT_EQUAL_CHAR(get<1>(lhs), 0xb);
  TEST_ASSERT_TRUE(get<2>(lhs));
}

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
