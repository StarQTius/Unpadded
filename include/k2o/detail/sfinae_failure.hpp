//! \file

//! \brief Defines a function template which displays an informative message at compile-time and stop compilation when
//! instanciated
#define K2O_SFINAE_FAILURE(FNAME, MESSAGE)                                                                             \
  template<bool __Sfinae_Failure = false>                                                                              \
  constexpr void FNAME(...) {                                                                                          \
    static_assert(__Sfinae_Failure, MESSAGE);                                                                          \
  }

//! \brief Defines a member function template which displays an informative message at compile-time and stop compilation
//! when instanciated
#define K2O_SFINAE_FAILURE_MEMBER(FNAME, MESSAGE)                                                                      \
  template<bool __Sfinae_Failure = false>                                                                              \
  constexpr void FNAME(...) const volatile {                                                                           \
    static_assert(__Sfinae_Failure, MESSAGE);                                                                          \
  }
