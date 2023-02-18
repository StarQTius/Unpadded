#include <upd/buffered_dispatcher.hpp>
#include <upd/keyring.hpp>
#include <upd/unevaluated.hpp>

#define UPD_TEST_SETUP

#include "utility.hpp"

int check_64(int x) {
  TEST_ASSERT_EQUAL(64, x);
  return {};
}

std::int64_t identity(std::int64_t x) { return x; }

void void_procedure() {}

constexpr auto kring =
    upd::make_keyring(upd::make_flist(UPD_CTREF(check_64), UPD_CTREF(identity), UPD_CTREF(void_procedure)),
                      upd::little_endian,
                      upd::two_complement);

static upd::action reply_hook;

void reply(const upd::byte_t (&payload)[16]) { reply_hook(std::begin(payload)); }
void reply_std_array(const std::array<upd::byte_t, 16> &payload) { reply_hook(payload.data()); }

constexpr auto reply_kring = upd::make_keyring(
    upd::make_flist(UPD_CTREF(reply), UPD_CTREF(reply_std_array)), upd::little_endian, upd::two_complement);

extern "C" void setUp() {}

extern "C" void tearDown() { reply_hook = {}; }

static void buffered_dispatcher_DO_load_an_action_EXPECT_correct_action_loaded_cpp17() {
#if __cplusplus >= 201703L
  using namespace upd;

  upd::byte_t kbuf[64];
  auto k = kring.get<check_64>();
  single_buffered_dispatcher dis{kring, policy::any_callback};

  k(64).write_to(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_from(kbuf));

  TEST_ASSERT_TRUE(dis.is_loaded());
#endif // __cplusplus >= 201703L
}

static void buffered_dispatcher_DO_load_an_action_in_a_single_buffered_dispatcher() {
  using namespace upd;

  upd::byte_t kbuf[64];
  int result = 0;
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_single_buffered_dispatcher(kring, policy::any_callback);

  static_assert(dis.buffer_size == sizeof(std::int64_t) + sizeof(decltype(dis)::index_t), "");

  k(64).write_to(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_from(kbuf));
  dis.write_to(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_EQUAL(64, result);
}

static void buffered_dispatcher_DO_load_an_action_in_a_double_buffered_dispatcher() {
  using namespace upd;

  upd::byte_t kbuf[64];
  int result = 0;
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_double_buffered_dispatcher(kring, policy::any_callback);

  static_assert(dis.input_buffer_size == sizeof(std::int64_t) + sizeof(decltype(dis)::index_t), "");
  static_assert(dis.output_buffer_size == sizeof(std::int64_t), "");

  k(64).write_to(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_from(kbuf));
  dis.write_to(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_EQUAL(64, result);
}

static void buffered_dispatcher_DO_load_an_action_in_a_double_buffered_dispatcher_while_already_loaded() {
  using namespace upd;

  upd::byte_t kbuf[64];
  std::int64_t result = 0;
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_double_buffered_dispatcher(kring, policy::any_callback);

  static_assert(dis.input_buffer_size == sizeof(std::int64_t) + sizeof(decltype(dis)::index_t), "");
  static_assert(dis.output_buffer_size == sizeof(std::int64_t), "");

  k(64).write_to(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_from(kbuf));

  TEST_ASSERT_TRUE(dis.is_loaded());

  k(32).write_to(kbuf);
  std::size_t i = 0;
  for (; i < sizeof(decltype(dis)::index_t); i++)
    dis.put(kbuf[i]);
  dis.write_to(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_FALSE(dis.is_loaded());
  TEST_ASSERT_EQUAL(64, result);

  for (; i < sizeof(decltype(dis)::index_t) + sizeof(std::int64_t); i++)
    dis.put(kbuf[i]);
  dis.write_to(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_EQUAL(32, result);
}

static void buffered_dispatcher_DO_replace_an_action() {
  using namespace upd;

  upd::byte_t kbuf[16];
  auto dis = make_single_buffered_dispatcher(kring, policy::any_callback);
  auto k = kring.get(UPD_CTREF(identity));

  dis.replace<1>([](std::int64_t x) -> std::int64_t {
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

  upd::byte_t kbuf[16];
  auto dis = make_single_buffered_dispatcher(kring, policy::any_callback);
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

static void buffered_dispatcher_DO_move_dispatcher_during_reading_and_writing() {
  using namespace upd;

  upd::byte_t kbuf[16] = {0};
  auto dis1 = make_single_buffered_dispatcher(kring, policy::any_callback);
  auto k = kring.get(UPD_CTREF(identity));

  k(64).write_to(kbuf);
  std::size_t i = 0;
  for (; i < sizeof(std::int64_t) / 2; i++)
    TEST_ASSERT_EQUAL(packet_status::LOADING_PACKET, dis1.put(kbuf[i]));

  auto dis2 = std::move(dis1);

  for (; i < sizeof(std::int64_t); i++)
    TEST_ASSERT_EQUAL(packet_status::LOADING_PACKET, dis2.put(kbuf[i]));

  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis2.put(kbuf[sizeof(std::int64_t)]));

  for (i = 0; i < sizeof(std::int64_t) / 2; i++) {
    TEST_ASSERT_TRUE(dis2.is_loaded());
    kbuf[i] = dis2.get();
  }

  dis1 = std::move(dis2);

  for (; i < sizeof(std::int64_t) - 1; i++) {
    TEST_ASSERT_TRUE(dis1.is_loaded());
    kbuf[i] = dis1.get();
  }

  TEST_ASSERT_TRUE(dis1.is_loaded());
  kbuf[sizeof(std::int64_t) - 1] = dis1.get();
  TEST_ASSERT_FALSE(dis1.is_loaded());

  TEST_ASSERT_EQUAL(64, k.read_from(kbuf));
}

static void buffered_dispatcher_DO_replace_void_procedure() {
  using namespace upd;

  upd::byte_t kbuf[16];
  auto dis = make_single_buffered_dispatcher(kring, policy::any_callback);
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

  upd::byte_t kbuf[16];
  auto dis = make_single_buffered_dispatcher(kring, policy::any_callback);
  auto k = kring.get(UPD_CTREF(identity));

  k(64) >> kbuf;

  auto *ptr = kbuf;

  packet_status status = dis.put(*ptr++);
  TEST_ASSERT_EQUAL(packet_status::LOADING_PACKET, status);
  while ((status = dis.put(*ptr++)) == packet_status::LOADING_PACKET)
    ;
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, status);

  ptr = kbuf;
  while (dis.is_loaded())
    *ptr++ = dis.get();
  TEST_ASSERT_EQUAL(64, k << kbuf);
}

static void buffered_dispatcher_DO_create_double_buffered_dispatcher_with_no_storage_action() {
  using namespace upd;

  upd::byte_t kbuf[64];
  int result = 0;
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_single_buffered_dispatcher(kring, policy::weak_reference);

  static_assert(dis.buffer_size == sizeof(std::int64_t) + sizeof(decltype(dis)::index_t), "");

  k(64).write_to(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis.read_from(kbuf));
  dis.write_to(reinterpret_cast<upd::byte_t *>(&result));

  TEST_ASSERT_EQUAL(64, result);
}

static void buffered_dispatcher_DO_reply() {
  using namespace upd;

  byte_t kbuf[64];
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_single_buffered_dispatcher(kring, policy::any_callback);
  auto reply_dis = make_single_buffered_dispatcher(reply_kring, policy::any_callback);

  std::int64_t result;
  reply_hook = k.with_hook([&](std::int64_t response) { result = response; });

  k(0xafbe).write_to(kbuf);
  dis.read_from(kbuf);
  dis.reply(kbuf, reply_kring.get(UPD_CTREF(reply)));

  reply_dis.read_from(kbuf);
  TEST_ASSERT_EQUAL(0xafbe, result);

  k(0xcddc).write_to(kbuf);
  dis.read_from(kbuf);
  dis.reply(kbuf, reply_kring.get(UPD_CTREF(reply_std_array)));

  reply_dis.read_from(kbuf);
  TEST_ASSERT_EQUAL(0xcddc, result);
}

static void buffered_dispatcher_DO_use_parenthesis_operator() {
  using namespace upd;

  upd::byte_t kbuf[64];
  auto k = kring.get(UPD_CTREF(identity));
  auto dis = make_single_buffered_dispatcher(kring, policy::any_callback);

  k(64).write_to(kbuf);
  TEST_ASSERT_EQUAL(packet_status::RESOLVED_PACKET, dis(kbuf, kbuf));
  TEST_ASSERT_EQUAL(64, k.read_from(kbuf));
}

int main() {
  using namespace upd;

  UNITY_BEGIN();
  RUN_TEST(buffered_dispatcher_DO_load_an_action_EXPECT_correct_action_loaded_cpp17);
  RUN_TEST(buffered_dispatcher_DO_load_an_action_in_a_single_buffered_dispatcher);
  RUN_TEST(buffered_dispatcher_DO_load_an_action_in_a_double_buffered_dispatcher);
  RUN_TEST(buffered_dispatcher_DO_load_an_action_in_a_double_buffered_dispatcher_while_already_loaded);
  RUN_TEST(buffered_dispatcher_DO_replace_an_action);
  RUN_TEST(buffered_dispatcher_DO_replace_void_procedure);
  RUN_TEST(buffered_dispatcher_DO_give_an_invalid_index);
  RUN_TEST(buffered_dispatcher_DO_move_dispatcher_during_reading_and_writing);
  RUN_TEST(buffered_dispatcher_DO_insert_bytes_one_by_one);
  RUN_TEST(buffered_dispatcher_DO_create_double_buffered_dispatcher_with_no_storage_action);
  RUN_TEST(buffered_dispatcher_DO_reply);
  RUN_TEST(buffered_dispatcher_DO_use_parenthesis_operator);
  return UNITY_END();
}
