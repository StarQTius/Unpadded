#pragma once

#include <concepts>

#include "constexpr.hpp"

namespace upd {

template<auto A, auto B>
concept equivalent_to = std::same_as<auto_constant<A>, auto_constant<B>>;

} // namespace upd
