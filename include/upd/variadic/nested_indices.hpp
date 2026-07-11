#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <utility>

namespace upd {

template<std::size_t... Ns>
struct nested_indices {
  constexpr static auto value = [] {
    namespace stdr = std::ranges;
    namespace stdv = std::views;

    using nested_index_type = std::pair<std::size_t, std::size_t>;

    constexpr auto size = (Ns + ... + 0uz);

    auto retval = std::array<nested_index_type, size>{};
    auto subsizes = std::array{Ns...};
    auto i = 0zu;
    auto it = retval.begin();
    for (auto ss : subsizes) {
      it = stdr::copy(stdv::repeat(i) | stdv::take(ss) | stdv::enumerate, it)
               .out;
      ++i;
    }

    return retval;
  }();
};

template<std::size_t... Ns>
constexpr auto nested_indices_v = nested_indices<Ns...>::value;

} // namespace upd
