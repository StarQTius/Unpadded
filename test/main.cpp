#include "main.hpp"
#include "tuple.hpp"
#include "tuple_view.hpp"
#include "unaligned_data.hpp"

// Check if private macros have been properly undefined
#ifdef FWD
#error "Private macros have leaked"
#endif

int main() {
  UNITY_BEGIN();

  run_tuple_ut();
  run_tuple_view_ut();
  run_unaligned_data_ut();

  return UNITY_END();
}
