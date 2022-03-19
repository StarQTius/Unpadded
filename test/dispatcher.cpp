#include <k2o/cpp11.hpp>
#include <k2o/dispatcher.hpp>

#include "utility.hpp"

int get_8() { return 8; }
int get_16() { return 16; }
int get_32() { return 32; }
int identity(int x) { return x; }

constexpr k2o::flist11_t<K2O_CTREF(get_8), K2O_CTREF(get_16), K2O_CTREF(get_32), K2O_CTREF(identity)> ftor_list;

static void dispatcher_DO_call_order_EXPECT_calling_correct_order() {
  using namespace k2o;

  constexpr auto kring = make_keyring(ftor_list);
  auto dispatcher = make_dispatcher(kring);
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

  make_dispatcher(kring)
      .get_order(input_f)
      .map([&](order &o) {
        o(input_f, output_f);
        TEST_ASSERT_EQUAL_UINT(16, output.get<0>());
      })
      .map_error([](size_t) { TEST_FAIL(); });
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(dispatcher_DO_call_order_EXPECT_calling_correct_order);
  RUN_TEST(dispatcher_DO_get_order_EXPECT_correct_index);
  return UNITY_END();
}
