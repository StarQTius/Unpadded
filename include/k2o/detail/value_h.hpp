//! \file
//! \brief Value representation through type

#pragma once

namespace k2o {
namespace detail {

//! \brief Holds a value
//! \tparam T Type of the held value
//! \tparam Value value to be held
template<typename T, T Value>
struct value_h {
  using type = T;
  constexpr static auto value = Value;
};

//! \brief Holds a value, without evaluating it
//! \tparam T type of the held value
//! \tparam Value value of the held value
template<typename T, T Value>
struct unevaluated_value_h {
  using type = T;
};

} // namespace detail
} // namespace k2o
