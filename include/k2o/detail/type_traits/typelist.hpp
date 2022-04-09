//! \file
//! \brief Variadic template manipulation

#pragma once

#include <cstddef>
#include <type_traits>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/detail/mp_list.hpp> // IWYU pragma: export
#include <boost/mp11/function.hpp>

// IWYU pragma: no_include "boost/mp11/detail/mp_min_element.hpp"
// IWYU pragma: no_include "boost/mp11/detail/mp_list.hpp"
// IWYU pragma: no_include "boost/mp11/detail/mp_plus.hpp"

namespace k2o {
namespace detail {

#if __cplusplus >= 201703L

//! \brief Compile-time index holder
template<std::size_t I>
using index_t = std::integral_constant<std::size_t, I>;

//! \copydoc index_t
template<std::size_t I>
index_t<I> index_v;

//! \brief Indexed element holder meant to be derived from
template<std::size_t I, typename T>
struct itype {
  using type = T;
  constexpr static auto value = I;

  constexpr static std::size_t index(T *) { return I; }
  static T element(index_t<I>);
};

//! \brief Allows efficient operations on template parameter packs
//!
//! Ts should expands to a list of `itype` instances.
template<typename... Ts>
struct tlist_t : Ts... {
  using Ts::index..., Ts::element...;
  template<typename T1, typename T2>
  constexpr tlist_t(T1, T2) {}
};

template<std::size_t... Is, template<typename...> typename L, typename... Ts>
tlist_t(std::index_sequence<Is...>, L<Ts...>) -> tlist_t<itype<Is, Ts>...>;

//! \copydoc tlist_t
template<typename... Ts>
constexpr tlist_t tlist{std::make_index_sequence<sizeof...(Ts)>{}, boost::mp11::mp_list<Ts...>{}};

//! \brief Convert a variadic template instance to a `tlist_t` instance
template<typename L>
constexpr tlist_t to_tlist{std::make_index_sequence<boost::mp11::mp_size<L>::value>{}, L{}};

//! \brief Return an instance of the Ith element a typelist
//!
//! This function must only be used in unevaluated context
template<std::size_t I, typename... Ts>
inline auto at_impl(tlist_t<Ts...> tl) {
  return tl.element(index_v<I>);
}

//! \brief Find the maximal value in a typelist of integer holders
template<typename... Ts>
constexpr inline auto max_impl(tlist_t<Ts...> tl) {
  constexpr long long vl[]{Ts::type::value...};
  auto max = vl[0];
  for (auto v : vl)
    max = max < v ? v : max;
  return max;
}

//! \brief Find the index of the given element in a typelist
//!
//! There must be only one element in the typelist aliasing the provided element.
template<typename T, typename... Ts>
constexpr inline auto find_impl(tlist_t<Ts...> tl) {
  return index_v<tl.index((T *)nullptr)>;
}

//! \brief Sum the values held by the typelist
template<typename... Ts>
constexpr auto sum_impl(tlist_t<Ts...>) {
  return std::integral_constant<decltype((0 + ... + Ts::type::value)), (0 + ... + Ts::type::value)>{};
}

//! \brief Expands to the element resulting in the call of `IMPL` on `LIST` converted to `tlist_t`
#define K2O_TLIST_FUNCTION(IMPL, LIST, ...) decltype(IMPL<__VA_ARGS__>(to_tlist<LIST>))

//! \brief Wraps the value of `IMPL` invoked on `LIST` converted to `tlist_t`
#define K2O_TLIST_WRAPPED_VALUE(IMPL, LIST, ...)                                                                       \
  std::integral_constant<decltype(IMPL<__VA_ARGS__>(to_tlist<LIST>)), IMPL<__VA_ARGS__>(to_tlist<LIST>)>

//! \name C++17 implementations
//! @{
#define K2O_AT_IMPL(L, I) K2O_TLIST_FUNCTION(at_impl, L, I)
#define K2O_MAX_IMPL(L) K2O_TLIST_WRAPPED_VALUE(max_impl, L)
#define K2O_FIND_IMPL(L, V) K2O_TLIST_FUNCTION(find_impl, L, V)
#define K2O_SUM_IMPL(L) K2O_TLIST_FUNCTION(sum_impl, L)
//! }@

#else // __cplusplus >= 201703L

//! \name Default implementations
//! @{
#define K2O_AT_IMPL(L, I) boost::mp11::mp_at_c<L, I>;
#define K2O_MAX_IMPL(L) boost::mp11::mp_max_element<L, boost::mp11::mp_less>;
#define K2O_FIND_IMPL(L, V) boost::mp11::mp_find<L, V>;
#define K2O_SUM_IMPL(L)                                                                                                \
  boost::mp11::mp_back<boost::mp11::mp_partial_sum<L, boost::mp11::mp_int<0>, boost::mp11::mp_plus>>
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

//! \copydoc map_impl
template<typename L, template<typename...> typename F>
using map = boost::mp11::mp_transform<F, L>;

//! \copydoc sum_impl
template<typename L>
using sum = K2O_SUM_IMPL(L);

//! \copydoc max
template<typename... Ts>
using max_p = max<boost::mp11::mp_list<Ts...>>;

} // namespace detail
} // namespace k2o
