#pragma once

#include <cstddef>
#include <type_traits>

#include "detail/integral_constant.hpp"

namespace upd {
namespace detail {

template<typename T, typename Integral_Constant_T>
struct at_most_t {
  using type = T;
  constexpr static auto max = Integral_Constant_T::value;
};

} // namespace detail

template<typename T, std::size_t Max>
using at_most = detail::at_most_t<T, detail::integral_constant_t<Max>>;

template<typename... Ts>
struct one_of {
  using alternative_ts = std::tuple<Ts...>;
  constexpr static auto alternative_count = sizeof...(Ts);
};

template<typename... Ts>
struct group {
  using types = std::tuple<Ts...>;
  constexpr static auto size = sizeof...(Ts);
};

} // namespace upd
