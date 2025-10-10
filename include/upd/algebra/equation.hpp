#pragma once

#include "concepts.hpp"

namespace upd::algebra {

template<expression Lhs, expression Rhs>
struct equation {
  Lhs lhs;
  Rhs rhs;
};

} // namespace upd::algebra
