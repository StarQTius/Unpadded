#pragma once

#include <tuple>

#include "clean.hpp"

namespace upd::detail::variadic {

template<typename, template<typename> typename>
struct filter;

template<typename... Ts, template<typename> typename F>
struct filter<std::tuple<Ts...>, F> {
  using marked = std::tuple<std::conditional_t<F<Ts>::value, Ts, marked_for_cleaning_t>...>;
  using type = clean_t<marked>;
};

template<typename Tuple, template<typename> typename F>
using filter_t = typename filter<Tuple, F>::type;

template<typename, typename>
struct filterf;

template<typename... Ts, typename F>
struct filterf<std::tuple<Ts...>, F> {
  using marked = std::tuple<std::conditional_t<std::invoke_result_t<F, Ts>::value, Ts, marked_for_cleaning_t>...>;
  using type = clean_t<marked>;
};

template<typename Tuple, typename F>
using filterf_t = typename filterf<Tuple, F>::type;

} // namespace upd::detail::variadic