#pragma once

#include <algorithm>
#include <cstddef>
#include <format>
#include <string_view>

#include "../upd.hpp"

namespace upd {

constexpr auto name_max_size = std::size_t{256};

struct name {
  template<std::size_t Size>
  consteval name(const char (&str)[Size]) noexcept(release) : string{} {
    using namespace std::ranges;

    copy(str, string);
  }

  consteval name(const char *str) noexcept(release) : string{} {
    using namespace std::ranges;

    copy(std::string_view{str}, string);
  }

  constexpr operator std::string_view() const noexcept(release) { return std::string_view{string}; }

  char string[name_max_size];
};

} // namespace upd

template<>
struct std::formatter<upd::name> {
  consteval formatter() noexcept(upd::release) = default;

  [[nodiscard]] constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  [[nodiscard]] constexpr static auto format(const upd::name &nm, std::format_context &ctx) {
    auto it = ctx.out();
    it = std::format_to(it, "{}", nm.string);

    ctx.advance_to(it);
    return it;
  }
};
