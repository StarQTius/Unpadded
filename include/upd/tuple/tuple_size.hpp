#pragma once

#include <type_traits>
#include <utility>

#include "tuple_like.hpp"

namespace upd {

template<tuple_like2 Tuple>
struct tuple_size {
  constexpr static auto value = std::tuple_size_v<std::remove_cvref_t<Tuple>>;
};

template<tuple_like2 Tuple>
constexpr auto tuple_size_v = tuple_size<Tuple>::value;

} // namespace upd
