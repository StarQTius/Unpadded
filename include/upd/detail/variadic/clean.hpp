#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <utility>

#include "at.hpp"

namespace upd::detail::variadic {

struct marked_for_cleaning_t {};

template<typename>
struct clean;

template<typename... Ts>
struct clean<std::tuple<Ts...>> {
  template<std::size_t... Is>
  [[nodiscard]] constexpr static auto kept_element_truth_array(std::index_sequence<Is...>) noexcept {
    std::array<bool, sizeof...(Ts)> retval{};

    ((retval.at(Is) = !std::is_same_v<marked_for_cleaning_t, Ts>), ...);

    return retval;
  }

  [[nodiscard]] constexpr static auto kept_element_count() noexcept {
    std::make_index_sequence<sizeof...(Ts)> iseq;

    constexpr auto truth_array = kept_element_truth_array(iseq);

    std::size_t retval = 0;
    for (auto kept : truth_array) {
      retval += kept ? 1 : 0;
    }

    return retval;
  }

  [[nodiscard]] constexpr static auto kept_element_indices() {
    std::make_index_sequence<sizeof...(Ts)> iseq;

    constexpr auto truth_array = kept_element_truth_array(iseq);
    constexpr auto kept_count = kept_element_count();

    std::array<std::size_t, kept_count> retval{};

    for (std::size_t retval_i = 0, truth_array_i = 0; retval_i < kept_count || truth_array_i < truth_array.size();
         ++truth_array_i) {
      if (truth_array.at(truth_array_i)) {
        retval.at(retval_i++) = truth_array_i;
      }
    }

    return retval;
  }

  template<std::size_t... Is>
  [[nodiscard]] constexpr static auto kept_element_tuple(std::index_sequence<Is...>) {
    std::make_index_sequence<sizeof...(Ts)> iseq;
    constexpr auto indices = kept_element_indices();

    return std::tuple<at_t<std::tuple<Ts...>, indices.at(Is)>...>{};
  }

  constexpr static std::make_index_sequence<kept_element_count()> cleaned_iseq{};
  using type = decltype(kept_element_tuple(cleaned_iseq));
};

template<typename T>
using clean_t = typename clean<T>::type;

} // namespace upd::detail::variadic
