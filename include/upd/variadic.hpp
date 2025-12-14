#pragma once

#include "constexpr.hpp"

namespace upd {

template<typename T>
concept variadic_instance =
    requires(T x) { []<template<typename...> typename TT, typename... Ts>(const TT<Ts...> &) {}(x); };

template<typename T>
using is_variadic_instance = auto_constant<variadic_instance<T>>;

template<typename T>
concept value_variadic_instance =
    requires(T x) { []<template<auto...> typename TT, auto... Values>(const TT<Values...> &) {}(x); };

template<typename T>
using is_value_variadic_instance = auto_constant<value_variadic_instance<T>>;

} // namespace upd
