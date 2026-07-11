#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <ranges>

namespace upd {

template<typename Cleanme, typename... Ts>
struct clean_occurences_of {
  constexpr static auto keeps_element =
      std::array<bool, sizeof...(Ts)>{!std::same_as<Ts, Cleanme>...};
  constexpr static auto kept_element_count =
      std::ranges::fold_left(keeps_element, 0uz, std::plus<std::size_t>{});
  constexpr static auto value = [] {
    namespace stdv = std::views;

    auto retval = std::array<std::size_t, kept_element_count>{};
    auto i = 0uz;
    for (auto [j, keep] : keeps_element | stdv::enumerate) {
      if (keep) {
        retval[i] = j;
        ++i;
      }
    }
    UPD_ASSERT(i == retval.size());

    return retval;
  }();
};

template<typename Cleanme, typename... Ts>
constexpr auto clean_occurences_of_v =
    clean_occurences_of<Cleanme, Ts...>::value;

} // namespace upd
