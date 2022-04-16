#include <k2o/dispatcher.hpp>
#include <k2o/keyring.hpp>
#include <k2o/unevaluated.hpp>

#include "utility.hpp"

int get_8() { return 8; }
int get_16() { return 16; }
int get_32() { return 32; }
int identity(int x) { return x; }

constexpr auto ftor_list = k2o::make_flist(K2O_CTREF(get_8), K2O_CTREF(get_16), K2O_CTREF(get_32), K2O_CTREF(identity));

static void dispatcher_DO_call_order_EXPECT_calling_correct_order() {
  using namespace k2o;

  constexpr auto kring = make_keyring(ftor_list);
  auto dispatcher = make_dispatcher(kring, policy::any_order);
  auto function16_index = upd::make_tuple(uint16_t{1});
  auto output = upd::make_tuple<int>();

  size_t i = 0, j = 0;
  dispatcher([&]() { return function16_index[i++]; }, [&](upd::byte_t byte) { output[j++] = byte; });

  TEST_ASSERT_EQUAL_UINT(16, output.get<0>());
}

static void dispatcher_DO_get_order_EXPECT_correct_index() {
  using namespace k2o;

  constexpr auto kring = make_keyring(ftor_list);
  auto function16_index = upd::make_tuple(uint16_t{1});
  auto output = upd::make_tuple<int>();

  size_t i = 0, j = 0;
  auto input_f = [&]() { return function16_index[i++]; };
  auto output_f = [&](upd::byte_t byte) { output[j++] = byte; };

  auto d = make_dispatcher(kring, policy::any_order);
  auto *order_ptr = d.get_order(input_f);
  TEST_ASSERT_NOT_NULL(order_ptr);

  (*order_ptr)(input_f, output_f);
  TEST_ASSERT_EQUAL_UINT(16, output.get<0>());
}

static void dispatcher_DO_call_no_storage_order_EXPECT_correct_behavior() {
  using namespace k2o;

  constexpr auto kring = make_keyring(ftor_list);
  auto dispatcher = make_dispatcher(kring, policy::static_storage_duration_only);
  auto function16_index = upd::make_tuple(uint16_t{1});
  auto output = upd::make_tuple<int>();

  size_t i = 0, j = 0;
  dispatcher([&]() { return function16_index[i++]; }, [&](upd::byte_t byte) { output[j++] = byte; });

  TEST_ASSERT_EQUAL_UINT(16, output.get<0>());
}

static void dispatcher_DO_replace_an_order_EXPECT_changed_order() {
  using namespace k2o;

  constexpr auto kring = make_keyring(ftor_list);
  auto dispatcher = make_dispatcher(kring, policy::any_order);
  auto function16_index = upd::make_tuple(uint16_t{1});
  auto output = upd::make_tuple<int>();

  size_t i = 0, j = 0;
  dispatcher([&]() { return function16_index[i++]; }, [&](upd::byte_t byte) { output[j++] = byte; });

  TEST_ASSERT_EQUAL_UINT(16, output.get<0>());

  i = 0, j = 0;
  dispatcher.replace<1>(get_32);
  dispatcher([&]() { return function16_index[i++]; }, [&](upd::byte_t byte) { output[j++] = byte; });

  TEST_ASSERT_EQUAL_UINT(32, output.get<0>());
}

static void dispatcher_DO_replace_a_no_storage_order_EXPECT_changed_order() {
  using namespace k2o;

  constexpr auto kring = make_keyring(ftor_list);
  auto dispatcher = make_dispatcher(kring, policy::static_storage_duration_only);
  auto function16_index = upd::make_tuple(uint16_t{1});
  auto output = upd::make_tuple<int>();

  size_t i = 0, j = 0;
  dispatcher([&]() { return function16_index[i++]; }, [&](upd::byte_t byte) { output[j++] = byte; });

  TEST_ASSERT_EQUAL_UINT(16, output.get<0>());

  i = 0, j = 0;
  dispatcher.replace<1>(K2O_CTREF(get_32));
  dispatcher([&]() { return function16_index[i++]; }, [&](upd::byte_t byte) { output[j++] = byte; });

  TEST_ASSERT_EQUAL_UINT(32, output.get<0>());
}

#define DISPATCHER make_dispatcher(make_keyring(ftor_list), policy::any_order)
#define DISPATCHER_STATIC make_dispatcher(make_keyring(ftor_list), policy::static_storage_duration_only)

int main() {
  using namespace k2o;

  DETECT(DISPATCHER(READABLE, WRITABLE),
         DISPATCHER(READABLE, BYTE_PTR),
         DISPATCHER(BYTE_PTR, WRITABLE),
         DISPATCHER(BYTE_PTR, BYTE_PTR),
         DISPATCHER.replace<0>(K2O_CTREF(FUNCTOR)),
         DISPATCHER.replace<0>(FUNCTOR),
         DISPATCHER_STATIC(READABLE, WRITABLE),
         DISPATCHER_STATIC(READABLE, BYTE_PTR),
         DISPATCHER_STATIC(BYTE_PTR, WRITABLE),
         DISPATCHER_STATIC(BYTE_PTR, BYTE_PTR),
         DISPATCHER_STATIC.replace<0>(K2O_CTREF(FUNCTOR)));

  UNITY_BEGIN();
  RUN_TEST(dispatcher_DO_call_order_EXPECT_calling_correct_order);
  RUN_TEST(dispatcher_DO_get_order_EXPECT_correct_index);
  RUN_TEST(dispatcher_DO_call_no_storage_order_EXPECT_correct_behavior);
  RUN_TEST(dispatcher_DO_replace_an_order_EXPECT_changed_order);
  RUN_TEST(dispatcher_DO_replace_a_no_storage_order_EXPECT_changed_order);
  return UNITY_END();
}
