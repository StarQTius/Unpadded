#pragma once

#include <cstddef>

namespace upd {

template<bool Is_Signed>
struct signedness_t {};

constexpr inline auto signed_int = signedness_t<true>{};
constexpr inline auto unsigned_int = signedness_t<false>{};

template<typename Enum>
struct enumeration_t {};

template<typename Enum>
constexpr inline auto enumeration = enumeration_t<Enum>{};

template<std::size_t Width>
struct width_t {};

template<std::size_t Width>
constexpr auto width = width_t<Width>{};

template<std::size_t Width>
struct at_most_t {};

template<std::size_t Width>
constexpr auto at_most = at_most_t<Width>{};

} // namespace upd
