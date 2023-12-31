#pragma once

#include <cstddef>

#include "../../index_type.hpp"

namespace upd::detail::variadic {

template<std::size_t I, typename T>
struct leaf {
  constexpr static auto at(index_type<I>) noexcept -> T *;
  constexpr static auto find(T *) noexcept -> index_type<I>;
};

} // namespace upd::detail::variadic
