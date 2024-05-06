#pragma once

#include <array>

namespace upd::detail {

template<auto... Values>
constexpr auto static_storage = std::array{Values...};

} // namespace upd::detail
