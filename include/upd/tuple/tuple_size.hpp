#pragma once

#include <type_traits>
#include <utility>

#include "tuple_like.hpp"

namespace upd {

template<tuple_like Tuple>
struct tuple_size {
  constexpr static auto value = std::tuple_size_v<std::remove_cvref_t<Tuple>>;
};

template<typename Tuple>
constexpr auto tuple_size_v = tuple_size<Tuple>::value;

} // namespace upd
