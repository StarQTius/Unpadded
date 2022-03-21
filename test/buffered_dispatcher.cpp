#include <k2o/buffered_dispatcher.hpp>
#include <k2o/cpp11.hpp>

#include "utility.hpp"

int check_64(int x) {
  TEST_ASSERT_EQUAL(64, x);
  return {};
}

constexpr auto kring = k2o::make_keyring(k2o::flist11_t<K2O_CTREF(check_64)>{});

static void buffered_dispatcher_DO_load_an_order_EXPECT_correct_order_loaded_cpp17() {
#if __cplusplus >= 201703L
  using namespace k2o;

  upd::byte_t kbuf[64], buf[64];
  auto k = kring.get<check_64>();
  buffered_dispatcher dis{kring, buf, buf};

  k(64).write_all(kbuf);
  dis.read_all(kbuf);

  TEST_ASSERT_TRUE(dis.is_loaded());
#endif // __cplusplus >= 201703L
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(buffered_dispatcher_DO_load_an_order_EXPECT_correct_order_loaded_cpp17);
  return UNITY_END();
}
