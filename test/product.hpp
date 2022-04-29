#pragma once

#include <upd/typelist.hpp>

template<typename>
struct pair_with_first;
template<typename T, typename... Ts>
struct pair_with_first<upd::typelist_t<T, Ts...>> {
  using type = upd::typelist_t<upd::typelist_t<T, Ts>...>;
};

template<typename, typename>
struct concat;
template<typename... Ts, typename... Us, typename... Ls>
struct concat<upd::typelist_t<upd::typelist_t<Ts...>, Ls...>, upd::typelist_t<Us...>>
    : concat<upd::typelist_t<Ls...>, upd::typelist_t<Us..., Ts...>> {};
template<typename... Us>
struct concat<upd::typelist_t<>, upd::typelist_t<Us...>> {
  using type = upd::typelist_t<Us...>;
};

template<typename, typename>
struct product;
template<typename... Ts, typename... Us>
struct product<upd::typelist_t<Ts...>, upd::typelist_t<Us...>> {
  using type = typename concat<upd::typelist_t<typename pair_with_first<upd::typelist_t<Ts, Us...>>::type...>,
                               upd::typelist_t<>>::type;
};
