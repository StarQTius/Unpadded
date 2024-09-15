#pragma once

#include <type_traits>

namespace upd::detail {

template<typename, template<typename...> typename>
struct is_instance_of : std::false_type {};

template<template<typename...> typename TT, typename... Ts>
struct is_instance_of<TT<Ts...>, TT> : std::true_type {};

template<typename T, template<typename...> typename TT>
constexpr auto is_instance_of_v = is_instance_of<T, TT>::value;

template<template<typename...> typename TT>
struct is_instance_of_partial {
  template<typename T>
  using type = is_instance_of<T, TT>;
};

} // namespace upd::detail
