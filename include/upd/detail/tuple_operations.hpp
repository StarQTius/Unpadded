#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "integral_constant.hpp"
#include "variadic/map.hpp"

namespace upd::detail {

template<typename T, T... Xs, typename F>
[[nodiscard]] constexpr auto transform_to_tuple(std::integer_sequence<T, Xs...>, F &&f) {
  return std::tuple<std::invoke_result_t<F, integral_constant_t<Xs>>...>{std::invoke(f, integral_constant_t<Xs>{})...};
}

template<typename T, T... Xs, typename F>
[[nodiscard]] constexpr auto transform_to_array(std::integer_sequence<T, Xs...>, F &&f) {
  using result_t = std::common_type_t<std::invoke_result_t<F, integral_constant_t<Xs>>...>;
  return std::array<result_t, sizeof...(Xs)>{std::invoke(f, integral_constant_t<Xs>{})...};
}

template<std::size_t Begin, std::size_t End, typename Tuple>
[[nodiscard]] constexpr auto subtuple(Tuple &tuple) noexcept {
  constexpr auto subtuple_size = End - Begin;
  constexpr auto seq = std::make_index_sequence<subtuple_size>{};

  auto get_element = [&](auto iconst) -> auto & { return std::get<Begin + iconst.value>(tuple); };

  return transform_to_tuple(seq, get_element);
}

template<typename Result_Ts, std::size_t... Is, typename F>
constexpr auto accumulate_tuple(std::index_sequence<Is...>, F &&f) {
  auto retval = detail::variadic::map_t<Result_Ts, std::optional>{};
  auto deref_opts = [](auto &...opts) {
    UPD_ASSERT((opts.has_value() && ...));
    return std::make_tuple(std::cref(*opts)...);
  };
  auto unwrap_opts = [](auto... opts) {
    UPD_ASSERT((opts.has_value() && ...));
    return std::tuple{*std::move(opts)...};
  };
  auto invoke_on_subtuple = [&](auto iconst) {
    auto subtpl = subtuple<0, iconst>(retval);
    auto ref_tuple = std::apply(deref_opts, subtpl);
    return std::invoke(f, ref_tuple);
  };

  ((std::get<Is>(retval) = invoke_on_subtuple(integral_constant_v<Is>)), ...);
  return std::apply(unwrap_opts, std::move(retval));
}

template<std::size_t... Is, typename F>
[[nodiscard]] constexpr auto transform_orderly_to_tuple(std::index_sequence<Is...> seq, F &&f) {
  using result_ts = std::tuple<std::invoke_result_t<F, integral_constant_t<Is>>...>;

  auto forward_tuple_size = [&](auto &&ref_tuple) {
    using ref_tuple_t = std::decay_t<decltype(ref_tuple)>;

    auto iconst = std::tuple_size<ref_tuple_t>{};
    return std::invoke(f, iconst);
  };

  return accumulate_tuple<result_ts>(seq, forward_tuple_size);
}

} // namespace upd::detail
