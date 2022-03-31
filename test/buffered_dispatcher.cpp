#include <k2o/buffered_dispatcher.hpp>
#include <k2o/cpp11.hpp>

#include "utility.hpp"

int check_64(int x) {
  TEST_ASSERT_EQUAL(64, x);
  return {};
}

int identity(int x) { return x; }

constexpr auto kring = k2o::make_keyring(k2o::flist11_t<K2O_CTREF(check_64), K2O_CTREF(identity)>{});

static void buffered_dispatcher_DO_load_an_order_EXPECT_correct_order_loaded_cpp17() {
#if __cplusplus >= 201703L
  using namespace k2o;

  upd::byte_t kbuf[64], buf[64];
  auto k = kring.get<check_64>();
  buffered_dispatcher dis{kring, buf, buf, policy::any_order};

  k(64).write_all(kbuf);
  dis.read_all(kbuf);

  TEST_ASSERT_TRUE(dis.is_loaded());
#endif // __cplusplus >= 201703L
}

static void buffered_dispatcher_DO_load_an_order_in_a_single_buffered_dispatcher() {
  using namespace k2o;

  upd::byte_t kbuf[64];
  int result = 0;
  auto k = kring.get<K2O_CTREF(identity)>();
  auto dis = make_single_buffered_dispatcher(kring, policy::any_order);

  static_assert(dis.buffer_size == sizeof(int) + sizeof(decltype(dis)::index_t));

  k(64).write_all(kbuf);
  dis.read_all(kbuf);
  dis.write_all(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_EQUAL(64, result);
}

#define BUFFERED_DISPATCHER make_buffered_dispatcher(kring, BYTE_PTR, BYTE_PTR, policy::any_order)
#define BUFFERED_DISPATCHER_STATIC                                                                                     \
  make_buffered_dispatcher(kring, BYTE_PTR, BYTE_PTR, policy::static_storage_duration_only)

int main() {
  using namespace k2o;

  DETECT(BUFFERED_DISPATCHER.read_all(BYTE_PTR),
         BUFFERED_DISPATCHER.read_all(READABLE),
         BUFFERED_DISPATCHER.read(BYTE_PTR),
         BUFFERED_DISPATCHER.read(READABLE),
         BUFFERED_DISPATCHER.read(REGISTRY),
         BUFFERED_DISPATCHER << BYTE_PTR,
         BUFFERED_DISPATCHER << READABLE,
         BUFFERED_DISPATCHER << REGISTRY,
         BUFFERED_DISPATCHER.write_all(BYTE_PTR),
         BUFFERED_DISPATCHER.write_all(WRITABLE),
         BUFFERED_DISPATCHER.write(BYTE_PTR),
         BUFFERED_DISPATCHER.write(WRITABLE),
         BUFFERED_DISPATCHER.write(REGISTRY),
         BUFFERED_DISPATCHER >> BYTE_PTR,
         BUFFERED_DISPATCHER >> WRITABLE,
         BUFFERED_DISPATCHER >> REGISTRY,
         BUFFERED_DISPATCHER.replace(0, K2O_CTREF(FUNCTOR){}),
         BUFFERED_DISPATCHER.replace(0, FUNCTOR),
         BUFFERED_DISPATCHER_STATIC.read_all(BYTE_PTR),
         BUFFERED_DISPATCHER_STATIC.read_all(READABLE),
         BUFFERED_DISPATCHER_STATIC.read(BYTE_PTR),
         BUFFERED_DISPATCHER_STATIC.read(READABLE),
         BUFFERED_DISPATCHER_STATIC.read(REGISTRY),
         BUFFERED_DISPATCHER_STATIC << BYTE_PTR,
         BUFFERED_DISPATCHER_STATIC << READABLE,
         BUFFERED_DISPATCHER_STATIC << REGISTRY,
         BUFFERED_DISPATCHER_STATIC.write_all(BYTE_PTR),
         BUFFERED_DISPATCHER_STATIC.write_all(WRITABLE),
         BUFFERED_DISPATCHER_STATIC.write(BYTE_PTR),
         BUFFERED_DISPATCHER_STATIC.write(WRITABLE),
         BUFFERED_DISPATCHER_STATIC.write(REGISTRY),
         BUFFERED_DISPATCHER_STATIC >> BYTE_PTR,
         BUFFERED_DISPATCHER_STATIC >> WRITABLE,
         BUFFERED_DISPATCHER_STATIC >> REGISTRY,
         BUFFERED_DISPATCHER_STATIC.replace(0, K2O_CTREF(FUNCTOR){}));

  UNITY_BEGIN();
  RUN_TEST(buffered_dispatcher_DO_load_an_order_EXPECT_correct_order_loaded_cpp17);
  RUN_TEST(buffered_dispatcher_DO_load_an_order_in_a_single_buffered_dispatcher);
  return UNITY_END();
}
