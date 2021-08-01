//! \file
//! \brief Value representation through type

#pragma once

#if __cplusplus >= 201703L
//! \brief Holds a value
//! \tparam T Type of the held value
//! \tparam Value value to be held
template<auto Value>
struct value_h { constexpr static auto value = Value; };
#endif// __cplusplus >= 201703L
