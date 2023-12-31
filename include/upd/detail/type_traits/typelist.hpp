//! \file

#pragma once

#include <cstddef>
#include <cstdint> // IWYU pragma: keep
#include <type_traits>

#include "ternary.hpp" // IWYU pragma: keep

// NOLINTBEGIN

#if __cplusplus >= 201703L
#include "index_sequence.hpp"
#endif // __cplusplus >= 201703L

// NOLINTNEXTLINE(modernize-concat-nested-namespaces)
namespace upd {
namespace detail {

#if __cplusplus >= 201703L

//! \brief Compile-time index holder
template<std::size_t I>
using index_t = std::integral_constant<std::size_t, I>;

//! \brief Compile-time type holder
template<typename>
struct element_t {};

//! \copydoc index_t
template<std::size_t I>
index_t<I> index_v;

//! \copydoc element_t
template<typename T>
element_t<T> element_v;

//! \brief Indexed element holder meant to be derived from
template<std::size_t I, typename T>
struct itype {
  using type = T;
  constexpr static auto value = I;

  constexpr static itype<I, T> index(element_t<T>) { return {}; };
  constexpr static itype<I, T> element(index_t<I>) { return {}; };
};

//! \name
//! \brief Allows efficient operations on template parameter packs
//!
//! `Ts` should expands to a list of `itype` instances.
//! @{
template<typename... Ts>
struct tlist_t_impl;
template<std::size_t... Is, typename... Ts>
struct tlist_t_impl<index_sequence<Is...>, Ts...> : itype<Is, Ts>... {
  using itype<Is, Ts>::index..., itype<Is, Ts>::element...;

  constexpr tlist_t_impl() = default;
  template<typename T1, typename T2>
  constexpr tlist_t_impl(T1, T2) {}
};
//! @}

//! \brief Allows efficient operations on template parameter packs
template<typename... Ts>
struct tlist_t : tlist_t_impl<make_index_sequence<sizeof...(Ts)>, Ts...> {
  using type = tlist_t<Ts...>;
};

//! \copydoc tlist_t
template<typename... Ts>
constexpr tlist_t<Ts...> tlist;

#else // __cplusplus >= 201703L

template<typename... Ts>
struct tlist_t {
  using type = tlist_t<Ts...>;
};

#endif // __cplusplus >= 201703L

//! \name
//! \brief Get the front element of a typelist
//! @{
template<typename>
struct front_impl;
template<typename T, typename... Ts>
struct front_impl<tlist_t<T, Ts...>> {
  using type = T;
};
//! @}

//! \name
//! \brief Pop the front element of a typelist
//! @{
template<typename>
struct pop_front_impl;
template<typename T, typename... Ts>
struct pop_front_impl<tlist_t<T, Ts...>> {
  using type = tlist_t<Ts...>;
};
//! @}

//! \name
//! \brief Append an element to a typelist
//! @{
template<typename, typename>
struct push_back_impl;
template<typename... Ts, typename T>
struct push_back_impl<tlist_t<Ts...>, T> {
  using type = tlist_t<Ts..., T>;
};
//! @}

//! \copydoc front_impl
template<typename L>
using front = typename front_impl<L>::type;

//! \copydoc pop_front_impl
template<typename L>
using pop_front = typename pop_front_impl<L>::type;

//! \copydoc push_back_impl
template<typename L, typename T>
using push_back = typename push_back_impl<L, T>::type;

#if __cplusplus >= 201703L

//! \brief Return an instance of the Ith element a typelist
template<std::size_t I, typename... Ts>
constexpr auto at_impl(tlist_t<Ts...> tl) {
  return tl.element(index_v<I>);
}

//! \brief Find the maximal value in a typelist of integer holders
template<typename... Ts>
constexpr auto max_impl(tlist_t<Ts...> tl) {
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
constexpr auto find_impl(tlist_t<Ts...> tl) {
  return index_v<tl.index(element_v<T>).value>;
}

//! \brief Sum the values held by the typelist
template<typename... Ts>
constexpr auto sum_impl(tlist_t<Ts...>) {
  return std::integral_constant<decltype((0 + ... + Ts::value)), (0 + ... + Ts::value)>{};
}

//! \brief Clip a subtypelist of length `sizeof...(Ns)` from `tl` starting from `I`
template<std::size_t I, typename... Ts, std::size_t... Ns>
constexpr auto clip_impl(tlist_t<Ts...> tl, index_sequence<Ns...>) {
  return tlist_t<typename decltype(at_impl<I + Ns>(tl))::type...>{};
}

//! \brief Expands to the element resulting in the call of `IMPL` on `LIST` converted to `tlist_t`
#define UPD_TLIST_FUNCTION(IMPL, LIST, ...) typename decltype(IMPL<__VA_ARGS__>(LIST{}))::type

//! \brief Wraps the value of `IMPL` invoked on `LIST` converted to `tlist_t`
#define UPD_TLIST_WRAPPED_VALUE(IMPL, LIST, ...)                                                                       \
  std::integral_constant<decltype(IMPL<__VA_ARGS__>(LIST{})), IMPL<__VA_ARGS__>(LIST{})>

//! \name C++17 implementations
//! @{
#define UPD_AT_IMPL(L, I) UPD_TLIST_FUNCTION(at_impl, L, I)
#define UPD_MAX_IMPL(L) UPD_TLIST_WRAPPED_VALUE(max_impl, L)
#define UPD_FIND_IMPL(L, V) UPD_TLIST_FUNCTION(find_impl, L, V)
#define UPD_SUM_IMPL(L) UPD_TLIST_FUNCTION(sum_impl, L)
#define UPD_CLIP_IMPL(L, I, N) decltype(clip_impl<I>(L{}, make_index_sequence<N>{}))
//! }@

#else // __cplusplus >= 201703L

//! \brief Return an instance of the Ith element a typelist
template<std::size_t, typename>
struct at_impl;
template<std::size_t I, typename T, typename... Ts>
struct at_impl<I, tlist_t<T, Ts...>> : at_impl<I - 1, tlist_t<Ts...>> {};
template<typename T, typename... Ts>
struct at_impl<0, tlist_t<T, Ts...>> {
  using type = T;
};

//! \brief Find the maximal value in a typelist of integer holders
template<typename>
struct max_impl;
template<typename Most, typename T, typename... Ts>
struct max_impl<tlist_t<Most, T, Ts...>> : max_impl<tlist_t<ternary_t<(T::value > Most::value), T, Most>, Ts...>> {};
template<typename Most>
struct max_impl<tlist_t<Most>> : Most {};

//! \brief Find the index of the first occurence of the given element in a typelist
template<typename, std::size_t, typename>
struct find_impl;
template<typename Target, std::size_t I, typename T, typename... Ts>
struct find_impl<Target, I, tlist_t<T, Ts...>> : find_impl<Target, I + 1, tlist_t<Ts...>> {};
template<typename Target, std::size_t I, typename... Ts>
struct find_impl<Target, I, tlist_t<Target, Ts...>> : std::integral_constant<std::size_t, I> {};

//! \brief Sum the values held by the typelist
template<typename, typename>
struct sum_impl;
template<typename Result, typename T, typename... Ts>
struct sum_impl<Result, tlist_t<T, Ts...>>
    : sum_impl<std::integral_constant<decltype(Result::value + T::value), Result::value + T::value>, tlist_t<Ts...>> {};
template<typename Result>
struct sum_impl<Result, tlist_t<>> : Result {};

//! \brief Clip a subtypelist of length `N` from `L` starting from `I`
template<std::size_t I, std::size_t N, typename L, typename Retval>
struct clip_impl : clip_impl<I - 1, N, pop_front<L>, Retval> {};
template<std::size_t N, typename L, typename Retval>
struct clip_impl<0, N, L, Retval> : clip_impl<0, N - 1, pop_front<L>, push_back<Retval, front<L>>> {};
template<typename L, typename Retval>
struct clip_impl<0, 0, L, Retval> {
  using type = Retval;
};

#define UPD_GET_TYPE(...) typename __VA_ARGS__::type

//! \name Default implementations
//! @{*
#define UPD_AT_IMPL(L, I) UPD_GET_TYPE(at_impl<I, typename L::type>)
#define UPD_MAX_IMPL(L) UPD_GET_TYPE(max_impl<typename L::type>)
#define UPD_FIND_IMPL(L, V) UPD_GET_TYPE(find_impl<V, 0, typename L::type>)
#define UPD_SUM_IMPL(L) UPD_GET_TYPE(sum_impl<std::integral_constant<std::uint8_t, 0>, typename L::type>)
#define UPD_CLIP_IMPL(L, I, N) UPD_GET_TYPE(clip_impl<I, N, typename L::type, tlist_t<>>)
//! }@

#endif // __cplusplus >= 201703L

//! \copydoc at_impl
template<typename L, std::size_t I>
using at = UPD_AT_IMPL(L, I);

//! \copydoc max_impl
template<typename L>
using max = UPD_MAX_IMPL(L);

//! \copydoc find_impl
template<typename L, typename V>
using find = UPD_FIND_IMPL(L, V);

//! \copydoc sum_impl
template<typename L>
using sum = UPD_SUM_IMPL(L);

//! \copydoc clip_impl
template<typename L, std::size_t I, std::size_t N>
using clip = UPD_CLIP_IMPL(L, I, N);

//! \copydoc max
template<typename... Ts>
using max_p = max<tlist_t<Ts...>>;

} // namespace detail
} // namespace upd

// NOLINTEND
