#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "tuple_like.hpp"

namespace upd {

template<std::size_t I, tuple_like2 Tuple>
struct tuple_element {
  using type = std::tuple_element_t<I, std::remove_cvref_t<Tuple>>;
};

template<std::size_t I, tuple_like2 Tuple>
using tuple_element_t = typename tuple_element<I, Tuple>::type;

} // namespace upd
