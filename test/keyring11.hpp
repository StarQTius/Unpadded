#pragma once

#include <unity.h>

#include <k2o/cpp11.hpp>
#include <k2o/keyring11.hpp>

void function1(int);
void function2(char);
int function3(long);

inline void keyring11_DO_get_an_ikey_EXPECT_correct_index() {
  k2o::keyring11<K2O_CTREF(function1), K2O_CTREF(function2), K2O_CTREF(function3)> keyring;
  auto ikey = keyring.get<K2O_CTREF(function2)>();

  TEST_ASSERT_EQUAL_UINT(1, ikey.index);
}
