#pragma once

#include <algorithm>
#include <cstddef>
#include <format>
#include <string>
#include <string_view>

#include "../upd.hpp"

namespace upd {

template<std::size_t N>
struct name {
  consteval name() noexcept(release) : string{} {}

  consteval name(const char (&str)[N]) noexcept(release) : string{} {
    using namespace std::ranges;

    copy(str, string);
  }

  constexpr auto anonymous() const noexcept(release) -> bool { return N == 0; }

  constexpr operator std::string_view() const noexcept(release) {
    return std::string_view{string};
  }

  char string[N];
};

constexpr auto anon = name<0>{};

template<std::size_t N, std::size_t M>
[[nodiscard]] constexpr inline auto
operator==(const name<N> &lhs, const name<M> &rhs) noexcept(release) -> bool {
  return std::string_view{lhs.string} == std::string_view{rhs.string};
};

template<std::size_t N>
[[nodiscard]] constexpr inline auto
operator==(const name<N> &lhs, std::string_view rhs) noexcept(release) -> bool {
  return std::string_view{lhs.string} == rhs;
};

template<std::size_t N>
[[nodiscard]] constexpr inline auto
operator==(std::string_view lhs, const name<N> &rhs) noexcept(release) -> bool {
  return lhs == std::string_view{rhs.string};
};

} // namespace upd

template<std::size_t N>
struct std::formatter<upd::name<N>> {
  consteval formatter() noexcept(upd::release) = default;

  [[nodiscard]] constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  [[nodiscard]] constexpr static auto
  format(const upd::name<N> &nm, std::format_context &ctx) {
    auto it = ctx.out();
    it = std::format_to(it, "{}", nm.string);

    ctx.advance_to(it);
    return it;
  }
};
