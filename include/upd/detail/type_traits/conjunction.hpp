//! \file

#pragma once

#include <cstddef> // IWYU pragma: keep
#include <type_traits>

namespace k2o {
namespace detail {

//! \name
//! \brief Forms the logical conjunction of the template parameters
//! @{

#if __cplusplus >= 201703L
template<typename... Ts>
using conjunction = std::integral_constant<bool, (Ts::value && ...)>;
#else  // __cplusplus >= 201703L
template<typename...>
struct conjunction;
template<typename T, typename... Ts>
struct conjunction<T, Ts...> : std::integral_constant<bool, T::value && conjunction<Ts...>::value> {};
template<>
struct conjunction<> : std::true_type {};
#endif // __cplusplus >= 201703L

//! @}

} // namespace detail
} // namespace k2o
