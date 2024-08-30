#pragma once

namespace upd {

template<bool Is_Signed>
struct signedness_t {};

constexpr inline auto signed_int = signedness_t<true>{};
constexpr inline auto unsigned_int = signedness_t<false>{};

template<std::size_t Width>
struct width_t {};

template<std::size_t Width>
constexpr auto width = width_t<Width>{};

} // namespace upd