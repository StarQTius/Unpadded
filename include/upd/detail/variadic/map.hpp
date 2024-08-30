#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "flatten.hpp"
#include "all_of.hpp"

namespace upd::detail::variadic {

template<typename, template<typename...> typename, typename...>
struct map;

template<typename... Ts, template<typename...> typename F, typename... Args>
struct map<std::tuple<Ts...>, F, Args...> {
  using type = std::tuple<F<Ts, Args...>...>;
};

template<typename T, template<typename...> typename F, typename... Args>
using map_t = typename map<T, F, Args...>::type;

template<typename, template<typename...> typename, typename...>
struct binmap;

template<typename... Pair_Ts, template<typename...> typename F, typename... Args>
struct binmap<std::tuple<Pair_Ts...>, F, Args...> {
  using type = std::tuple<F<std::tuple_element_t<0, Pair_Ts>, std::tuple_element_t<1, Pair_Ts>, Args...>...>;
};

template<typename T, template<typename...> typename F, typename... Args>
using binmap_t = typename binmap<T, F, Args...>::type;

template<typename, typename>
struct mapf;

template<typename... Ts, typename F>
struct mapf<std::tuple<Ts...>, F> {
  using type = std::tuple<std::invoke_result_t<F, Ts>...>;
};

template<typename T, typename F>
using mapf_t = typename mapf<T, F>::type;

template<typename, typename>
struct flatmapf;

template<typename... Ts, typename F>
struct flatmapf<std::tuple<Ts...>, F> {
  using result_t = mapf_t<std::tuple<Ts...>, F>;

  using type = flatten_t<result_t>;
};

template<typename T, typename F>
using flatmapf_t = typename mapf<T, F>::type;

template<typename Tuple, typename F>
constexpr void for_each(Tuple, F) {
  using are_voids = map_t<Tuple, std::is_void>;
  
  static_assert(all_of_v<are_voids>, "`F` should always return `void`");
}

} // namespace upd::detail::variadic
