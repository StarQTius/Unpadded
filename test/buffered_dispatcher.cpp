#include <upd/buffered_dispatcher.hpp>
#include <upd/keyring.hpp>
#include <upd/unevaluated.hpp>

#include "utility.hpp"

int check_64(int x) {
  TEST_ASSERT_EQUAL(64, x);
  return {};
}

int identity(int x) { return x; }

void void_procedure() {}

constexpr auto kring =
    upd::make_keyring(upd::make_flist(UPD_CTREF(check_64), UPD_CTREF(identity), UPD_CTREF(void_procedure)));

static void buffered_dispatcher_DO_load_an_action_EXPECT_correct_action_loaded_cpp17() {
#if __cplusplus >= 201703L
  using namespace upd;

  upd::byte_t kbuf[64], buf[64];
  auto k = kring.get<check_64>();
  buffered_dispatcher dis{kring, buf, buf, policy::any_action};

  k(64).write_all(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_all(kbuf));

  TEST_ASSERT_TRUE(dis.is_loaded());
#endif // __cplusplus >= 201703L
}

static void buffered_dispatcher_DO_load_an_action_in_a_single_buffered_dispatcher() {
  using namespace upd;

  upd::byte_t kbuf[64];
  int result = 0;
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_single_buffered_dispatcher(kring, policy::any_action);

  static_assert(dis.buffer_size == sizeof(int) + sizeof(decltype(dis)::index_t));

  k(64).write_all(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_all(kbuf));
  dis.write_all(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_EQUAL(64, result);
}

static void buffered_dispatcher_DO_load_an_action_in_a_double_buffered_dispatcher() {
  using namespace upd;

  upd::byte_t kbuf[64];
  int result = 0;
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_double_buffered_dispatcher(kring, policy::any_action);

  static_assert(dis.input_buffer_size == sizeof(int) + sizeof(decltype(dis)::index_t));
  static_assert(dis.output_buffer_size == sizeof(int));

  k(64).write_all(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_all(kbuf));
  dis.write_all(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_EQUAL(64, result);
}

static void buffered_dispatcher_DO_load_an_action_in_a_double_buffered_dispatcher_while_already_loaded() {
  using namespace upd;

  upd::byte_t kbuf[64];
  int result = 0;
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_double_buffered_dispatcher(kring, policy::any_action);

  static_assert(dis.input_buffer_size == sizeof(int) + sizeof(decltype(dis)::index_t));
  static_assert(dis.output_buffer_size == sizeof(int));

  k(64).write_all(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_all(kbuf));

  TEST_ASSERT_TRUE(dis.is_loaded());

  k(32).write_all(kbuf);
  std::size_t i = 0;
  for (; i < sizeof(decltype(dis)::index_t); ++i)
    dis.read(kbuf + i);
  dis.write_all(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_FALSE(dis.is_loaded());
  TEST_ASSERT_EQUAL(64, result);

  for (; i < sizeof(decltype(dis)::index_t) + sizeof(int); ++i)
    dis.read(kbuf + i);
  dis.write_all(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_EQUAL(32, result);
}

static void buffered_dispatcher_DO_replace_an_action() {
  using namespace upd;

  upd::byte_t buf[16], kbuf[16];
  auto dis = make_buffered_dispatcher(kring, buf, buf, policy::any_action);
  auto k = kring.get(UPD_CTREF(identity));

  dis.replace<1>([](int x) {
    TEST_ASSERT_EQUAL(x, 64);
    return 32;
  });
  k(64) >> kbuf;
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis << kbuf);
  dis >> kbuf;
  TEST_ASSERT_EQUAL(k << kbuf, 32);
}

static void buffered_dispatcher_DO_give_an_invalid_index() {
  using namespace upd;

  upd::byte_t buf[16], kbuf[16];
  auto dis = make_buffered_dispatcher(kring, buf, buf, policy::any_action);
  auto k = kring.get(UPD_CTREF(identity));

  k(64) >> kbuf;
  kbuf[0] = 0xff;

  TEST_ASSERT_EQUAL(packet_status::DROPPED_PACKET, dis << kbuf);
  TEST_ASSERT_FALSE(dis.is_loaded());

  k(64) >> kbuf;
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis << kbuf);
  TEST_ASSERT_TRUE(dis.is_loaded());
  dis >> kbuf;
  TEST_ASSERT_EQUAL(64, k << kbuf);
}

static void buffered_dispatcher_DO_replace_void_procedure() {
  using namespace upd;

  upd::byte_t buf[16], kbuf[16];
  auto dis = make_buffered_dispatcher(kring, buf, buf, policy::any_action);
  auto k = kring.get(UPD_CTREF(void_procedure));

  bool flag = false;
  dis.replace<2>([&]() { flag = true; });

  k() >> kbuf;
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis << kbuf);
  dis >> kbuf;
  TEST_ASSERT_TRUE(flag);
}

static void buffered_dispatcher_DO_insert_bytes_one_by_one() {
  using namespace upd;

  upd::byte_t buf[16], kbuf[16];
  auto dis = make_buffered_dispatcher(kring, buf, buf, policy::any_action);
  auto k = kring.get(UPD_CTREF(identity));

  k(64) >> kbuf;

  auto *ptr = kbuf;

  packet_status status = dis.read(ptr++);
  TEST_ASSERT_EQUAL(packet_status::LOADING_PACKET, status);
  while ((status = dis.read(ptr++)) == packet_status::LOADING_PACKET)
    ;
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, status);

  ptr = kbuf;
  while (dis.is_loaded())
    dis.write(ptr++);
  TEST_ASSERT_EQUAL(64, k << kbuf);
}

static void buffered_dispatcher_DO_create_double_buffered_dispatcher_with_no_storage_action() {
  using namespace upd;

  upd::byte_t kbuf[64];
  int result = 0;
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_single_buffered_dispatcher(kring, policy::static_storage_duration_only);

  static_assert(dis.buffer_size == sizeof(int) + sizeof(decltype(dis)::index_t));

  k(64).write_all(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_all(kbuf));
  dis.write_all(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_EQUAL(64, result);
}

#define BUFFERED_DISPATCHER make_buffered_dispatcher(kring, BYTE_PTR, BYTE_PTR, policy::any_action)
#define BUFFERED_DISPATCHER_STATIC                                                                                     \
  make_buffered_dispatcher(kring, BYTE_PTR, BYTE_PTR, policy::static_storage_duration_only)

int main() {
  using namespace upd;

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
         BUFFERED_DISPATCHER.replace<0>(UPD_CTREF(FUNCTOR)),
         BUFFERED_DISPATCHER.replace<0>(FUNCTOR),
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
         BUFFERED_DISPATCHER_STATIC.replace<0>(UPD_CTREF(FUNCTOR)));

  UNITY_BEGIN();
  RUN_TEST(buffered_dispatcher_DO_load_an_action_EXPECT_correct_action_loaded_cpp17);
  RUN_TEST(buffered_dispatcher_DO_load_an_action_in_a_single_buffered_dispatcher);
  RUN_TEST(buffered_dispatcher_DO_load_an_action_in_a_double_buffered_dispatcher);
  RUN_TEST(buffered_dispatcher_DO_load_an_action_in_a_double_buffered_dispatcher_while_already_loaded);
  RUN_TEST(buffered_dispatcher_DO_replace_an_action);
  RUN_TEST(buffered_dispatcher_DO_replace_void_procedure);
  RUN_TEST(buffered_dispatcher_DO_give_an_invalid_index);
  RUN_TEST(buffered_dispatcher_DO_insert_bytes_one_by_one);
  RUN_TEST(buffered_dispatcher_DO_create_double_buffered_dispatcher_with_no_storage_action);
  return UNITY_END();
}
