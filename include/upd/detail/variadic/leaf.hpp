#pragma once

#include <cstddef>

#include "../../index_type.hpp"

namespace upd::detail::variadic {

template<std::size_t I, typename T>
struct leaf {
  constexpr static T *at(index_type<I>) noexcept;
  constexpr static index_type<I> find(T *) noexcept;
};

} // namespace upd::detail::variadic
