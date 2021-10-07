#include "keyring11.hpp"

void function1(int);
void function2(char);
int function3(long);

void keyring11_DO_get_an_ikey_EXPECT_correct_index() {
  using namespace k2o;

  keyring11<upd::endianess::BUILTIN,
            upd::signed_mode::BUILTIN,
            K2O_CTREF(function1),
            K2O_CTREF(function2),
            K2O_CTREF(function3)>
      kring;
  auto k = kring.get<K2O_CTREF(function2)>();

  TEST_ASSERT_EQUAL_UINT(1, k.index);
}
