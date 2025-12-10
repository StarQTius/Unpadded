#pragma once

#include <type_traits>
#include <utility>

namespace upd {

template<typename Tuple>
struct tuple_size {
  constexpr static auto value = std::tuple_size_v<std::remove_cvref_t<Tuple>>;
};

template<typename Tuple>
constexpr auto tuple_size_v = tuple_size<Tuple>::value;

} // namespace upd
