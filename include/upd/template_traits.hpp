#pragma once

namespace upd::detail {

template<template<typename...> typename, typename>
struct instantiate_variadic;

template<template<typename...> typename TT, template<typename...> typename UT, typename... Ts>
struct instantiate_variadic<TT, UT<Ts...>> {
  using type = TT<Ts...>;
};

} // namespace upd::detail

namespace upd {

template<typename T, template<typename...> typename TT>
concept instance_of = requires(T x) { []<typename... Ts>(const TT<Ts...> &) {} (x); };

template<typename T>
concept instance_of_variadic = requires(T x) { 
  []<template<typename...> typename TT, typename... Ts>(const TT<Ts...> &) {} (x);
};

template<template<typename...> typename TT, instance_of_variadic Args>
using instantiate_variadic = typename detail::instantiate_variadic<TT, Args>::type;

} // namespace upd
