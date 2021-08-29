//! \file
//! \brief Compile-time index finding

#pragma once

#include <boost/mp11.hpp>
#include <boost/type_traits.hpp>

#include "value_h.hpp"

namespace k2o {
namespace detail {

#ifdef DOXYGEN
//! \brief Helper class for finding the index of a value in a pack
//! \details
//!   'find_in_typelist<T, Value, Typelist>::value' is the position of
//!   'detail::value_h<T, Value>' in 'Typelist'.
//! \attention
//!   Please avoid using this alias if you are not compiling in C++17 at least, for it uses recursive
//!   template under the hood.
//! \tparam Value value to be found
//! \tparam Typelist typelist to search
template<typename T, typename Typelist>
struct find_in_typelist;
#else // DOXYGEN
#if __cplusplus >= 201703L
template<size_t Index, typename T>
struct pack_introspector {
  using singleton_t = boost::mp11::mp_list<T>;
  constexpr static auto get_index(singleton_t) { return Index; };
};

template<typename, typename>
struct find_in_typelist_impl;

template<typename... Ts, size_t... Is>
struct find_in_typelist_impl<boost::mp11::mp_list<Ts...>, boost::mp11::index_sequence<Is...>>
    : pack_introspector<Is, Ts>... {
  using pack_introspector<Is, Ts>::get_index...;
  constexpr static void get_index(...) {}
};

template<typename T, typename Typelist>
constexpr size_t find_in_typelist() {
  using namespace boost;

  constexpr auto typelist_size = mp11::mp_size<Typelist>::value;
  using singleton_t = mp11::mp_list<T>;
  using finder_t = find_in_typelist_impl<Typelist, mp11::make_index_sequence<typelist_size>>;

  static_assert(!is_void<decltype(finder_t::get_index(singleton_t{}))>::value,
                "The provided type does not belong to this typelist");

  return finder_t::get_index(singleton_t{});
};
#else  // __cplusplus >= 201703L
template<typename T, typename Typelist>
constexpr size_t find_in_typelist() {
  static_assert(boost::mp11::mp_find<Typelist, T>::value != boost::mp11::mp_size<Typelist>::value,
                "The provided type does not belong to this typelist");
  return boost::mp11::mp_find<Typelist, T>::value;
};
#endif // __cplusplus >= 201703L
#endif // DOXYGEN

} // namespace detail
} // namespace k2o
