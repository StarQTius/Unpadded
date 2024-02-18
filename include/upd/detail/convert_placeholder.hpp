#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <variant>

#include "../placeholder.hpp"
#include "../static_vector.hpp"

namespace upd::detail {

template<typename T>
struct convert_placeholder {
  using type = T;
};

template<typename T, std::size_t N>
struct convert_placeholder<T[N]> {
  using type = std::array<typename convert_placeholder<T>::type, N>;
};

template<typename T, std::size_t Max>
struct convert_placeholder<at_most<T, Max>> {
  using type = static_vector<typename convert_placeholder<T>::type, Max>;
};

template<typename... Ts>
struct convert_placeholder<one_of<Ts...>> {
  using type = std::variant<typename convert_placeholder<Ts>::type...>;
};

template<typename... Ts>
struct convert_placeholder<group<Ts...>> {
  using type = std::tuple<typename convert_placeholder<Ts>::type...>;
};

template<typename T>
using convert_placeholder_t = typename convert_placeholder<T>::type;

} // namespace upd::detail
