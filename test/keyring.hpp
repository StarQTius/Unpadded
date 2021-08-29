#pragma once

#include <unity.h>

#include <k2o/keyring.hpp>

void function1(int);
void function2(char);
int function3(long);

inline void keyring_DO_get_an_ikey_EXPECT_correct_index_cpp17() {
#if __cplusplus >= 201703L
  k2o::keyring<function1, function2, function3> keyring;
  auto ikey = keyring.get<function2>();

  TEST_ASSERT_EQUAL_UINT(1, ikey.index);
#endif // __cplusplus >= 201703L
}
