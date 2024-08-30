#pragma once

#include <type_traits>

namespace upd::detail::variadic {

template<typename, typename>
struct diff;

template<typename... Ts, typename... Us>
struct diff<std::tuple<Ts...>, std::tuple<Us...>> {
    constexpr static auto min_count = std::min(sizeof...(Ts), sizeof...(Us));
    constexpr static auto max_count = std::max(sizeof...(Ts), sizeof...(Us));

    using lhs = std::tuple<Ts...>;
    using clipped_lhs = clip_t<lhs, 0, min_count>;

    using rhs = std::tuple<Us...>;
    using clipped_rhs = clip_t<rhs, 0, min_count>;

    template<typename... _Ts, typename... _Us, std::size_t... Is>
    [[nodiscard]] static auto impl(std::tuple<_Ts...>, std::tuple<_Us...>, std::index_sequence<Is...>) noexcept {
        return std::tuple<std::is_same<_Ts, _Us>..., std::bool_constant<true || Is>...>{};
    }

    using type = decltype(impl(std::declval<lhs>(), std::declval<rhs>(), std::make_index_sequence<max_count - min_count>{}));
};

template<typename Lhs, typename Rhs>
using diff_t = typename diff<Lhs, Rhs>::type;

} // namespace upd::detail::variadic