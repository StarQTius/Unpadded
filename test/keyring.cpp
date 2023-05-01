#include <upd/keyring.hpp>
#include <upd/unevaluated.hpp>

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

constexpr auto ftor_list = upd::make_flist(UPD_CTREF(ftor3),
                                           UPD_CTREF(function1),
                                           UPD_CTREF(function2),
                                           UPD_CTREF(ftor1),
                                           UPD_CTREF(function3),
                                           UPD_CTREF(ftor2));

static void keyring_DO_get_an_ikey_EXPECT_correct_index() {
  using namespace upd;

  constexpr auto kring = make_keyring(
      make_flist(UPD_CTREF(function1), UPD_CTREF(function2), UPD_CTREF(function3)), little_endian, twos_complement);
  auto k = kring.get(UPD_CTREF(function2));

  TEST_ASSERT_EQUAL_UINT(1, k.index);
}

static void keyring_DO_use_make_keyring_EXPECT_correct_behavior() {
  using namespace upd;

  constexpr auto kring = make_keyring(ftor_list, little_endian, twos_complement);
  auto k = kring.get(UPD_CTREF(function2));

  TEST_ASSERT_EQUAL_UINT(2, k.index);
}

static void keyring_DO_get_an_ikey_EXPECT_correct_index_cpp17() {
#if __cplusplus >= 201703L
  using namespace upd;

  constexpr auto ftor_list = flist<function1, ftor1, function2, ftor2, ftor3, function3>;
  keyring kring{ftor_list, upd::little_endian, upd::twos_complement};
  auto k = kring.get<function2>();

  TEST_ASSERT_EQUAL_UINT(2, k.index);
#endif // __cplusplus >= 201703L
}

static void keyring_DO_get_an_ikey_by_variable_EXPECT_correct_index_cpp17() {
#if __cplusplus >= 201703L
  using namespace upd;

  constexpr auto ftor_list = flist<function1, ftor1, function2, ftor2, ftor3, function3>;
  keyring kring{ftor_list, little_endian, twos_complement};
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
