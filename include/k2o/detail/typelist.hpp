//! \file
//! \brief Variadic template manipulation

#pragma once

#include <cstddef>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/function.hpp>

// IWYU pragma: no_include "boost/mp11/detail/mp_min_element.hpp"

namespace k2o {
namespace detail {

#if __cplusplus >= 201703L

//! \brief Compile-time index holder
template<std::size_t I>
using index_t = std::integral_constant<std::size_t, I>;

//! \copydoc index_t
template<std::size_t I>
index_t<I> index_v;

//! \brief Indexed type holder meant to be derived from
template<std::size_t I, typename T>
struct itype {
  constexpr static std::size_t index(T *) { return I; }
  static T type(index_t<I>);
};

//! \brief Allows efficient operations on template parameter packs
//! \details
//!   Ts should expands to a list of 'itype' instances.
template<typename... Ts>
struct tlist_t : Ts... {
  using Ts::index..., Ts::type...;
  template<typename T1, typename T2>
  tlist_t(T1, T2) {}
};

template<std::size_t... Is, typename... Ts>
tlist_t(std::index_sequence<Is...>, boost::mp11::mp_list<Ts...>) -> tlist_t<itype<Is, Ts>...>;

//! \copydoc tlist_t
template<typename... Ts>
tlist_t tlist{std::make_index_sequence<sizeof...(Ts)>{}, boost::mp11::mp_list<Ts...>{}};

//! \brief Converts a variadic template instance into an equivalent instance of 'tlist_t'
template<typename>
struct to_tlist_t;

//! \copydoc to_tlist
template<template<typename...> typename L, typename... Ts>
struct to_tlist_t<L<Ts...>> : decltype(tlist<Ts...>) {};

//! \copydoc to_tlist_t
template<typename L>
to_tlist_t<L> to_tlist;

//! \brief Return an instance of the Ith type a typelist
//! \details
//!   This function must only be used in unevaluated context
template<std::size_t I, typename... Ts>
inline auto at_impl(tlist_t<Ts...> tl) {
  return tl.type(index_v<I>);
}

//! \brief Find the maximal value in a typelist of integer holders
template<typename... Ts>
constexpr inline auto max_impl(tlist_t<Ts...> tl) {
  constexpr long long vl[]{Ts::value...}, max = vl[0];
  for (auto v : vl)
    max = max < v ? v : max;
  return max;
}

//! \brief Find the index of the given type in a typelist
//! \details
//!   There must be only one element in the typelist aliasing the provided type.
template<typename T, typename... Ts>
constexpr inline auto find_impl(tlist_t<Ts...> tl) {
  return index_v<tl.index((T *)nullptr)>;
}

//! \brief Expands to the type resulting in the call of IMPL on LIST converted to 'tlist_t'
#define K2O_TLIST_FUNCTION(IMPL, LIST, ...) decltype(IMPL<__VA_ARGS__>(to_tlist<LIST>))

//! \name C++17 implementations
//! @{
#define K2O_AT_IMPL(L, I) K2O_TLIST_FUNCTION(at_impl, L, I)
#define K2O_MAX_IMPL(L) K2O_TLIST_FUNCTION(max_impl, L)
#define K2O_FIND_IMPL(L, V) K2O_TLIST_FUNCTION(find_impl, L, V)
//! }@

#else // __cplusplus >= 201703L

//! \name Default implementations
//! @{
#define K2O_AT_IMPL(L, I) boost::mp11::mp_at_c<L, I>;
#define K2O_MAX_IMPL(L) boost::mp11::mp_max_element<L, boost::mp11::mp_less>;
#define K2O_FIND_IMPL(L, V) boost::mp11::mp_find<L, V>;
//! }@

#endif // __cplusplus >= 201703L

//! \copydoc at_impl
template<typename L, std::size_t I>
using at = K2O_AT_IMPL(L, I);

//! \copydoc max_impl
template<typename L>
using max = K2O_MAX_IMPL(L);

//! \copydoc find_impl
template<typename L, typename V>
using find = K2O_FIND_IMPL(L, V);

} // namespace detail
} // namespace k2o
