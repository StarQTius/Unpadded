#pragma once

#include <tuple>
#include <type_traits>

namespace upd::detail::variadic {

template<typename, template<typename...> typename, typename...>
struct map;

template<typename... Ts, template<typename...> typename F, typename... Args>
struct map<std::tuple<Ts...>, F, Args...> {
  using type = std::tuple<F<Ts, Args...>...>;
};

template<typename T, template<typename...> typename F, typename... Args>
using map_t = typename map<T, F, Args...>::type;

template<typename, typename>
struct mapf;

template<typename... Ts, typename F>
struct mapf<std::tuple<Ts...>, F> {
  using type = std::tuple<std::invoke_result_t<F, Ts>...>;
};

template<typename T, typename F>
using mapf_t = typename mapf<T, F>::type;

} // namespace upd::detail::variadic
