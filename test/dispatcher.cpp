#include "dispatcher.hpp"

int get_8() { return 8; }
int get_16() { return 16; }
int get_32() { return 32; }
int identity(int x) { return x; }

void dispatcher_DO_call_order_EXPECT_calling_correct_order() {
  using namespace k2o;

  keyring11<upd::endianess::BUILTIN,
            upd::signed_mode::BUILTIN,
            K2O_CTREF(get_8),
            K2O_CTREF(get_16),
            K2O_CTREF(get_32),
            K2O_CTREF(identity)>
      kring;
  auto dispatcher = make_dispatcher(kring);
  auto function16_index = upd::make_tuple(uint16_t{1});
  auto output = upd::make_tuple<int>();

  size_t i = 0, j = 0;
  dispatcher([&]() { return function16_index[i++]; }, [&](upd::byte_t byte) { output[j++] = byte; });

  TEST_ASSERT_EQUAL_UINT(16, output.get<0>());
}
