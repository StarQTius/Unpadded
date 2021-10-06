#include "keyring.hpp"

template<typename R, typename... Args>
struct callable {
  R operator()(Args &&...) { return {}; }
  // This line force the object not to be considered constexpr implicitely
  int x;
};

void function1(int);
void function2(char);
int function3(long);

extern callable<int, int> ftor1;
extern callable<int, long> ftor2;
extern callable<void, int> ftor3;

void keyring_DO_get_an_ikey_EXPECT_correct_index_cpp17() {
#if __cplusplus >= 201703L
  using namespace k2o;

  keyring<function1, ftor1, function2, ftor2, ftor3, function3> kring;
  auto k = kring.get<function2>();

  TEST_ASSERT_EQUAL_UINT(2, k.index);
#endif // __cplusplus >= 201703L
}

void keyring_DO_get_an_ikey_by_variable_EXPECT_correct_index_cpp17() {
#if __cplusplus >= 201703L
  using namespace k2o;

  keyring<function1, ftor1, function2, ftor2, ftor3, function3> kring;
  auto k = kring.get<ftor3>();

  TEST_ASSERT_EQUAL_UINT(4, k.index);
#endif // __cplusplus >= 201703L
}
