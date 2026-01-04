#pragma once

#include "../upd.hpp"

namespace upd::variadic {

template<template<auto, typename> typename TT, typename... Args>
[[nodiscard]] constexpr auto is_template_deductible_from() noexcept(release) -> bool {
  return requires(Args &&...args) { TT{UPD_FWD(args)...}; };
}

} // namespace upd::variadic
