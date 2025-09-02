#pragma once

#include "constexpr.hpp"
#include "lite_tuple.hpp"
#include "template_traits.hpp"
#include "type_traits.hpp"

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

template<typename T, typename Variadic>
concept element_of =
    variadic_instance<Variadic> && instantiate_variadic<detail::lite_tuple, Variadic>::has_type(typebox<T>{});

template<typename T, variadic_instance Variadic>
using is_element_of = auto_constant<element_of<T, Variadic>>;

template<auto Value, typename Variadic>
concept value_element_of =
    value_variadic_instance<Variadic> && instantiate_variadic<detail::lite_tuple, Variadic>::has_type(expr<Value>);

template<auto Value, value_variadic_instance Variadic>
using is_value_element_of = auto_constant<value_element_of<Value, Variadic>>;

} // namespace upd
