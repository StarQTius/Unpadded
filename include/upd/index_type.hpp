#pragma once

#include <cstddef>
#include <type_traits>

namespace upd {

template<std::size_t I>
using index_type = std::integral_constant<std::size_t, I>;

template<std::size_t I>
constexpr inline index_type<I> index_type_v;

} // namespace upd
