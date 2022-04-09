//! \file

//! \brief Defines a function template which displays an informative message at compile-time and stop compilation when
//! instanciated
#define K2O_SFINAE_FAILURE(FNAME, MESSAGE)                                                                             \
  template<bool __Sfinae_Failure = false>                                                                              \
  constexpr int FNAME(...) {                                                                                           \
    static_assert(__Sfinae_Failure, MESSAGE);                                                                          \
    return {};                                                                                                         \
  }

//! \brief Defines a member function template which displays an informative message at compile-time and stop compilation
//! when instanciated
#define K2O_SFINAE_FAILURE_MEMBER(FNAME, MESSAGE)                                                                      \
  template<bool __Sfinae_Failure = false>                                                                              \
  constexpr int FNAME(...) const volatile {                                                                            \
    static_assert(__Sfinae_Failure, MESSAGE);                                                                          \
    return {};                                                                                                         \
  }

//! \name
//! \brief Group of predefined error message
//! @{

#define K2O_ERROR_NOT_INPUT(X)                                                                                         \
  BACK_QUOTE(#X) " is neither an invocable with returns a byte when invoked nor an input iterator to a byte sequence"
#define K2O_ERROR_NOT_OUTPUT(X)                                                                                        \
  BACK_QUOTE(#X) " is neither invocable on a byte nor an output iterator to a byte sequence"
#define K2O_ERROR_NOT_BYTE_ITERATOR(X) BACK_QUOTE(#X) " is not an iterator to a byte sequence"

//! @}
