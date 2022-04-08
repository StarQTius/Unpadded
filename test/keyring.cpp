#include <k2o/cpp11.hpp>
#include <k2o/keyring.hpp>

#include "utility.hpp"

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

constexpr auto ftor_list = k2o::make_flist(K2O_CTREF(ftor3),
                                           K2O_CTREF(function1),
                                           K2O_CTREF(function2),
                                           K2O_CTREF(ftor1),
                                           K2O_CTREF(function3),
                                           K2O_CTREF(ftor2));

static void keyring_DO_get_an_ikey_EXPECT_correct_index() {
  using namespace k2o;

  constexpr auto kring = make_keyring(make_flist(K2O_CTREF(function1), K2O_CTREF(function2), K2O_CTREF(function3)));
  auto k = kring.get(K2O_CTREF(function2));

  TEST_ASSERT_EQUAL_UINT(1, k.index);
}

static void keyring_DO_use_make_keyring_EXPECT_correct_behavior() {
  using namespace k2o;

  constexpr auto kring = make_keyring(ftor_list);
  auto k = kring.get(K2O_CTREF(function2));

  TEST_ASSERT_EQUAL_UINT(2, k.index);
}

static void keyring_DO_get_an_ikey_EXPECT_correct_index_cpp17() {
#if __cplusplus >= 201703L
  using namespace k2o;

  constexpr auto ftor_list = flist<function1, ftor1, function2, ftor2, ftor3, function3>;
  keyring kring{ftor_list, upd::little_endian, upd::two_complement};
  auto k = kring.get<function2>();

  TEST_ASSERT_EQUAL_UINT(2, k.index);
#endif // __cplusplus >= 201703L
}

static void keyring_DO_get_an_ikey_by_variable_EXPECT_correct_index_cpp17() {
#if __cplusplus >= 201703L
  using namespace k2o;

  constexpr auto ftor_list = flist<function1, ftor1, function2, ftor2, ftor3, function3>;
  keyring kring{ftor_list};
  auto k = kring.get<ftor3>();

  TEST_ASSERT_EQUAL_UINT(4, k.index);
#endif // __cplusplus >= 201703L
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(keyring_DO_get_an_ikey_EXPECT_correct_index);
  RUN_TEST(keyring_DO_use_make_keyring_EXPECT_correct_behavior);
  RUN_TEST(keyring_DO_get_an_ikey_EXPECT_correct_index_cpp17);
  RUN_TEST(keyring_DO_get_an_ikey_by_variable_EXPECT_correct_index_cpp17);
  return UNITY_END();
}
