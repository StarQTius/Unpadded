#pragma once

#include "with_sequence.hpp"

#ifdef UPD_READABLE_CONCEPT_DIAGNOSTIC

#define UPD_ALL_OF_CONCEPT(CONCEPT, T, N, ...)                                 \
  (N < 63 && UPD_ALL_OF_CONCEPT_0(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_0(CONCEPT, T, N, ...)                               \
  (((N <= 0) || CONCEPT<T, 0 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_1(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_1(CONCEPT, T, N, ...)                               \
  (((N <= 1) || CONCEPT<T, 1 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_2(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_2(CONCEPT, T, N, ...)                               \
  (((N <= 2) || CONCEPT<T, 2 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_3(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_3(CONCEPT, T, N, ...)                               \
  (((N <= 3) || CONCEPT<T, 3 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_4(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_4(CONCEPT, T, N, ...)                               \
  (((N <= 4) || CONCEPT<T, 4 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_5(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_5(CONCEPT, T, N, ...)                               \
  (((N <= 5) || CONCEPT<T, 5 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_6(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_6(CONCEPT, T, N, ...)                               \
  (((N <= 6) || CONCEPT<T, 6 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_7(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_7(CONCEPT, T, N, ...)                               \
  (((N <= 7) || CONCEPT<T, 7 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_8(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_8(CONCEPT, T, N, ...)                               \
  (((N <= 8) || CONCEPT<T, 8 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_9(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_9(CONCEPT, T, N, ...)                               \
  (((N <= 9) || CONCEPT<T, 9 __VA_OPT__(, ) __VA_ARGS__>)                      \
   && UPD_ALL_OF_CONCEPT_10(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_10(CONCEPT, T, N, ...)                              \
  (((N <= 10) || CONCEPT<T, 10 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_11(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_11(CONCEPT, T, N, ...)                              \
  (((N <= 11) || CONCEPT<T, 11 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_12(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_12(CONCEPT, T, N, ...)                              \
  (((N <= 12) || CONCEPT<T, 12 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_13(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_13(CONCEPT, T, N, ...)                              \
  (((N <= 13) || CONCEPT<T, 13 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_14(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_14(CONCEPT, T, N, ...)                              \
  (((N <= 14) || CONCEPT<T, 14 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_15(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_15(CONCEPT, T, N, ...)                              \
  (((N <= 15) || CONCEPT<T, 15 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_16(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_16(CONCEPT, T, N, ...)                              \
  (((N <= 16) || CONCEPT<T, 16 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_17(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_17(CONCEPT, T, N, ...)                              \
  (((N <= 17) || CONCEPT<T, 17 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_18(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_18(CONCEPT, T, N, ...)                              \
  (((N <= 18) || CONCEPT<T, 18 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_19(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_19(CONCEPT, T, N, ...)                              \
  (((N <= 19) || CONCEPT<T, 19 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_20(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_20(CONCEPT, T, N, ...)                              \
  (((N <= 20) || CONCEPT<T, 20 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_21(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_21(CONCEPT, T, N, ...)                              \
  (((N <= 21) || CONCEPT<T, 21 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_22(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_22(CONCEPT, T, N, ...)                              \
  (((N <= 22) || CONCEPT<T, 22 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_23(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_23(CONCEPT, T, N, ...)                              \
  (((N <= 23) || CONCEPT<T, 23 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_24(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_24(CONCEPT, T, N, ...)                              \
  (((N <= 24) || CONCEPT<T, 24 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_25(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_25(CONCEPT, T, N, ...)                              \
  (((N <= 25) || CONCEPT<T, 25 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_26(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_26(CONCEPT, T, N, ...)                              \
  (((N <= 26) || CONCEPT<T, 26 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_27(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_27(CONCEPT, T, N, ...)                              \
  (((N <= 27) || CONCEPT<T, 27 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_28(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_28(CONCEPT, T, N, ...)                              \
  (((N <= 28) || CONCEPT<T, 28 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_29(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_29(CONCEPT, T, N, ...)                              \
  (((N <= 29) || CONCEPT<T, 29 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_30(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_30(CONCEPT, T, N, ...)                              \
  (((N <= 30) || CONCEPT<T, 30 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_31(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_31(CONCEPT, T, N, ...)                              \
  (((N <= 31) || CONCEPT<T, 31 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_32(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_32(CONCEPT, T, N, ...)                              \
  (((N <= 32) || CONCEPT<T, 32 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_33(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_33(CONCEPT, T, N, ...)                              \
  (((N <= 33) || CONCEPT<T, 33 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_34(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_34(CONCEPT, T, N, ...)                              \
  (((N <= 34) || CONCEPT<T, 34 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_35(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_35(CONCEPT, T, N, ...)                              \
  (((N <= 35) || CONCEPT<T, 35 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_36(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_36(CONCEPT, T, N, ...)                              \
  (((N <= 36) || CONCEPT<T, 36 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_37(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_37(CONCEPT, T, N, ...)                              \
  (((N <= 37) || CONCEPT<T, 37 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_38(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_38(CONCEPT, T, N, ...)                              \
  (((N <= 38) || CONCEPT<T, 38 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_39(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_39(CONCEPT, T, N, ...)                              \
  (((N <= 39) || CONCEPT<T, 39 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_40(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_40(CONCEPT, T, N, ...)                              \
  (((N <= 40) || CONCEPT<T, 40 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_41(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_41(CONCEPT, T, N, ...)                              \
  (((N <= 41) || CONCEPT<T, 41 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_42(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_42(CONCEPT, T, N, ...)                              \
  (((N <= 42) || CONCEPT<T, 42 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_43(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_43(CONCEPT, T, N, ...)                              \
  (((N <= 43) || CONCEPT<T, 43 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_44(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_44(CONCEPT, T, N, ...)                              \
  (((N <= 44) || CONCEPT<T, 44 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_45(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_45(CONCEPT, T, N, ...)                              \
  (((N <= 45) || CONCEPT<T, 45 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_46(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_46(CONCEPT, T, N, ...)                              \
  (((N <= 46) || CONCEPT<T, 46 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_47(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_47(CONCEPT, T, N, ...)                              \
  (((N <= 47) || CONCEPT<T, 47 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_48(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_48(CONCEPT, T, N, ...)                              \
  (((N <= 48) || CONCEPT<T, 48 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_49(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_49(CONCEPT, T, N, ...)                              \
  (((N <= 49) || CONCEPT<T, 49 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_50(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_50(CONCEPT, T, N, ...)                              \
  (((N <= 50) || CONCEPT<T, 50 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_51(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_51(CONCEPT, T, N, ...)                              \
  (((N <= 51) || CONCEPT<T, 51 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_52(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_52(CONCEPT, T, N, ...)                              \
  (((N <= 52) || CONCEPT<T, 52 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_53(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_53(CONCEPT, T, N, ...)                              \
  (((N <= 53) || CONCEPT<T, 53 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_54(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_54(CONCEPT, T, N, ...)                              \
  (((N <= 54) || CONCEPT<T, 54 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_55(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_55(CONCEPT, T, N, ...)                              \
  (((N <= 55) || CONCEPT<T, 55 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_56(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_56(CONCEPT, T, N, ...)                              \
  (((N <= 56) || CONCEPT<T, 56 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_57(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_57(CONCEPT, T, N, ...)                              \
  (((N <= 57) || CONCEPT<T, 57 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_58(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_58(CONCEPT, T, N, ...)                              \
  (((N <= 58) || CONCEPT<T, 58 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_59(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_59(CONCEPT, T, N, ...)                              \
  (((N <= 59) || CONCEPT<T, 59 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_60(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_60(CONCEPT, T, N, ...)                              \
  (((N <= 60) || CONCEPT<T, 60 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_61(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_61(CONCEPT, T, N, ...)                              \
  (((N <= 61) || CONCEPT<T, 61 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_62(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_62(CONCEPT, T, N, ...)                              \
  (((N <= 62) || CONCEPT<T, 62 __VA_OPT__(, ) __VA_ARGS__>)                    \
   && UPD_ALL_OF_CONCEPT_63(CONCEPT, T, N __VA_OPT__(, ) __VA_ARGS__))

#define UPD_ALL_OF_CONCEPT_63(CONCEPT, T, N, ...)                              \
  ((N <= 63) || CONCEPT<T, 63 __VA_OPT__(, ) __VA_ARGS__>)

#else // UPD_READABLE_CONCEPT_DIAGNOSTIC

#define UPD_ALL_OF_CONCEPT(CONCEPT, T, N, ...)                                 \
  UPD_WITH_SEQUENCE(IS, N) {                                                   \
    return (CONCEPT<T, IS __VA_OPT__(, ) __VA_ARGS__> && ...);                 \
  }

#endif // UPD_READABLE_CONCEPT_DIAGNOSTIC
