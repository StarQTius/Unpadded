#pragma once

#include "with_sequence.hpp"

#ifdef UPD_READABLE_CONCEPT_DIAGNOSTIC

#define UPD_ALL_OF_CONCEPT(CONCEPT, T, N, ...)                                                                         \
  (N < 16 && UPD_ALL_OF_CONCEPT_15(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_0(CONCEPT, T, N, ...) ((N == 0) || CONCEPT<T, 0 __VA_OPT__(, ) __VA_ARGS__>)

#define UPD_ALL_OF_CONCEPT_1(CONCEPT, T, N, ...)                                                                       \
  (((N <= 1) || CONCEPT<T, 1 __VA_OPT__(, ) __VA_ARGS__>) &&                                                           \
   UPD_ALL_OF_CONCEPT_0(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_2(CONCEPT, T, N, ...)                                                                       \
  (((N <= 2) || CONCEPT<T, 2 __VA_OPT__(, ) __VA_ARGS__>) &&                                                           \
   UPD_ALL_OF_CONCEPT_1(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_3(CONCEPT, T, N, ...)                                                                       \
  (((N <= 3) || CONCEPT<T, 3 __VA_OPT__(, ) __VA_ARGS__>) &&                                                           \
   UPD_ALL_OF_CONCEPT_2(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_4(CONCEPT, T, N, ...)                                                                       \
  (((N <= 4) || CONCEPT<T, 4 __VA_OPT__(, ) __VA_ARGS__>) &&                                                           \
   UPD_ALL_OF_CONCEPT_3(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_5(CONCEPT, T, N, ...)                                                                       \
  (((N <= 5) || CONCEPT<T, 5 __VA_OPT__(, ) __VA_ARGS__>) &&                                                           \
   UPD_ALL_OF_CONCEPT_4(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_6(CONCEPT, T, N, ...)                                                                       \
  (((N <= 6) || CONCEPT<T, 6 __VA_OPT__(, ) __VA_ARGS__>) &&                                                           \
   UPD_ALL_OF_CONCEPT_5(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_7(CONCEPT, T, N, ...)                                                                       \
  (((N <= 7) || CONCEPT<T, 7 __VA_OPT__(, ) __VA_ARGS__>) &&                                                           \
   UPD_ALL_OF_CONCEPT_6(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_8(CONCEPT, T, N, ...)                                                                       \
  (((N <= 8) || CONCEPT<T, 8 __VA_OPT__(, ) __VA_ARGS__>) &&                                                           \
   UPD_ALL_OF_CONCEPT_7(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_9(CONCEPT, T, N, ...)                                                                       \
  (((N <= 9) || CONCEPT<T, 9 __VA_OPT__(, ) __VA_ARGS__>) &&                                                           \
   UPD_ALL_OF_CONCEPT_8(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_10(CONCEPT, T, N, ...)                                                                      \
  (((N <= 10) || CONCEPT<T, 10 __VA_OPT__(, ) __VA_ARGS__>) &&                                                         \
   UPD_ALL_OF_CONCEPT_9(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_11(CONCEPT, T, N, ...)                                                                      \
  (((N <= 11) || CONCEPT<T, 11 __VA_OPT__(, ) __VA_ARGS__>) &&                                                         \
   UPD_ALL_OF_CONCEPT_10(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_12(CONCEPT, T, N, ...)                                                                      \
  (((N <= 12) || CONCEPT<T, 12 __VA_OPT__(, ) __VA_ARGS__>) &&                                                         \
   UPD_ALL_OF_CONCEPT_11(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_13(CONCEPT, T, N, ...)                                                                      \
  (((N <= 13) || CONCEPT<T, 13 __VA_OPT__(, ) __VA_ARGS__>) &&                                                         \
   UPD_ALL_OF_CONCEPT_12(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_14(CONCEPT, T, N, ...)                                                                      \
  (((N <= 14) || CONCEPT<T, 14 __VA_OPT__(, ) __VA_ARGS__>) &&                                                         \
   UPD_ALL_OF_CONCEPT_13(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_15(CONCEPT, T, N, ...)                                                                      \
  (((N <= 15) || CONCEPT<T, 15 __VA_OPT__(, ) __VA_ARGS__>) &&                                                         \
   UPD_ALL_OF_CONCEPT_14(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#else // UPD_READABLE_CONCEPT_DIAGNOSTIC

#define UPD_ALL_OF_CONCEPT(CONCEPT, T, N, ...)                                                                         \
  UPD_WITH_SEQUENCE(IS, N) { return (CONCEPT<T, IS __VA_OPT__(, ) __VA_ARGS__> && ...); }

#endif // UPD_READABLE_CONCEPT_DIAGNOSTIC
