#pragma once

#include <tuple>

namespace upd::detail::variadic {

template<typename, typename>
struct count;

template<typename... Ts, typename U>
struct count<std::tuple<Ts...>, U> {
  constexpr static auto value = (std::is_same_v<Ts, U> + ... + 0);
};

template<typename T, typename U>
constexpr auto count_v = count<T, U>::value;

} // namespace upd::detail::variadic
