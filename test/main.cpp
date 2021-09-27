#include "main.hpp"

int main() {
  UNITY_BEGIN();

  run_tuple_ut();
  run_unaligned_data_ut();

  return UNITY_END();
}
