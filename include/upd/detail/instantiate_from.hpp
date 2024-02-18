#pragma once

#include <tuple>

namespace upd::detail {

template<template<typename...> typename, typename>
struct instantiate_from;

template<template<typename...> typename TT, typename... Ts>
struct instantiate_from<TT, std::tuple<Ts...>> {
  using type = TT<Ts...>;
};

template<template<typename...> typename TT, typename Tuple_T>
using instantiate_from_t = typename instantiate_from<TT, Tuple_T>::type;

} // namespace upd::detail
