//! \file

#pragma once

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

//! \brief Defines a constructor function template which displays an informative message at compile-time and stop
//! compilation when instanciated
#define K2O_SFINAE_FAILURE_CTOR(CNAME, MESSAGE)                                                                        \
  template<bool __Sfinae_Failure = false>                                                                              \
  constexpr explicit CNAME(...) {                                                                                      \
    static_assert(__Sfinae_Failure, MESSAGE);                                                                          \
  }

//! \name
//! \brief Group of predefined error message
//! @{

#define K2O_ERROR_NOT_INPUT(X)                                                                                         \
  "`" #X "` is neither an invocable with returns a byte when invoked nor an input iterator to a byte sequence"
#define K2O_ERROR_NOT_OUTPUT(X) "`" #X "` is neither invocable on a byte nor an output iterator to a byte sequence"
#define K2O_ERROR_NOT_BYTE_ITERATOR(X) "`" #X "` is not an iterator to a byte sequence"
#define K2O_ERROR_NOT_INVOCABLE(X) "`" #X "` is not invocable"
#define K2O_ERROR_NOT_ALL_INVOCABLE(X) "There is one or more element(s) in `" #X "` which are not invocable objects"
#define K2O_ERROR_NOT_ALL_LVALUE(X) "There is one or more element(s) in `" #X "` which are not lvalues"
#define K2O_ERROR_NOT_KEYRING(X) "`" #X "` is not a template instance of `keyring`"

//! @}
