#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <format>
#include <ranges>
#include <string_view>
#include <tuple>

#include "../collector_of.hpp"
#include "../constexpr.hpp"
#include "../get.hpp"
#include "../tuple/join.hpp"
#include "../tuple/to.hpp"
#include "../tuple/transform.hpp"
#include "../tuple/tuple_like.hpp"
#include "../tuple/tuple_size.hpp"
#include "../upd.hpp"
#include "../variadic/template_box.hpp"
#include "../with_sequence.hpp"

namespace upd {

constexpr auto name_max_size = std::size_t{256};

struct name {
  constexpr name() noexcept(release) : anonymous{true}, string{} {}

  template<std::size_t Size>
  consteval name(const char (&str)[Size]) noexcept(release) : anonymous{false}, string{} {
    using namespace std::ranges;

    copy(str, string);
  }

  consteval name(const char *str) noexcept(release) : anonymous{false}, string{} {
    using namespace std::ranges;

    copy(std::string_view{str}, string);
  }

  constexpr operator std::string_view() const noexcept(release) { return std::string_view{string}; }

  bool anonymous;
  char string[name_max_size];
};

constexpr auto anon = name{};

[[nodiscard]] constexpr inline auto operator==(const name &lhs, const name &rhs) noexcept(release) -> bool {
  return std::string_view{lhs.string} == std::string_view{rhs.string};
};

template<std::size_t N>
struct names {
  constexpr static auto size = N;

  template<std::convertible_to<std::string_view>... Strings>
    requires(sizeof...(Strings) == N)
  consteval names(Strings... strs) noexcept(release) : strings{} {
    using namespace std::ranges;
    auto i = std::size_t{0};
    (copy(std::string_view{strs}, strings[i++]), ...);
  }

  char strings[N][name_max_size];
};

template<>
struct names<0> {
  constexpr static auto size = 0;
  constexpr static char *strings[0] = {};
};

template<std::convertible_to<std::string_view>... Strings>
names(Strings...) -> names<sizeof...(Strings)>;

template<names... NameLists>
concept unique_names = sizeof...(NameLists) > 0 && [] {
  namespace stdr = std::ranges;
  namespace updv = upd::tuple_views;

  auto to_sv_array = [](auto &name_list) {
    if constexpr (name_list.size == 0) {
      return std::array<std::string_view, 0>{};
    } else {
      auto sv_array = std::array<std::string_view, name_list.size>{};
      auto last = stdr::copy(name_list.strings, sv_array.begin());

      UPD_CONSTEXPR_ASSERT(last.out == sv_array.end());

      return sv_array;
    }
  };

  auto joined_names = std::tuple{NameLists...} | updv::transform(to_sv_array) | updv::join | updv::to<std::array>;

  stdr::sort(joined_names);
  return stdr::adjacent_find(joined_names) == joined_names.end();
}();

} // namespace upd

template<std::size_t N>
struct upd::tuple_like_for<upd::names<N>> {
  constexpr static auto size = N;

  template<std::size_t I>
  using element_type = char[name_max_size];

  template<std::size_t I, typename Names>
  [[nodiscard]] constexpr static auto get(Names &&ns) noexcept(release) -> auto && {
    return UPD_FWD(ns).strings[I];
  }
};

template<>
struct upd::collector_for<upd::template_box<upd::names>> {
  template<tuple_like2 View>
  [[nodiscard]] constexpr static auto collect(View &&view) {
    return UPD_WITH_SEQUENCE(Is, tuple_size_v<View>, &) { return names<sizeof...(Is)>{get<Is>(UPD_FWD(view))...}; };
  }
};

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
