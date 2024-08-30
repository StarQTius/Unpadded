#pragma once

namespace upd::detail::variadic {

template<typename>
struct to_array;

template<typename... Ts>
struct to_array<std::tuple<Ts...>> {
    constexpr static auto value = std::array{Ts::value...};
};

template<typename Tuple>
constexpr auto to_array_v = to_array<Tuple>::value;

} // namespace upd::detail::variadic