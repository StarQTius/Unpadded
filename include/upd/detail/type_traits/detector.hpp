//! \file

#pragma once

//! \brief Make a detector function which implements the Detection Idiom
//!
//! A detector is a template of `constexpr` functions which return a `bool` value wrapped in a `std::integral_constant`.
//! This value is `true` if and only if the `TEMPLATE_PARAMETERS` satisfy the provided `REQUIREMENTS`. `REQUIREMENTS`
//! are SFINAE requirements.
#define K2O_DETAIL_MAKE_DETECTOR(NAME, TEMPLATE_PARAMETERS, REQUIREMENTS)                                              \
  template<TEMPLATE_PARAMETERS, REQUIREMENTS>                                                                          \
  std::true_type NAME(int);                                                                                            \
  template<TEMPLATE_PARAMETERS>                                                                                        \
  std::false_type NAME(...);
