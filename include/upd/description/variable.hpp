#pragma once

#include <cstddef>
#include <format>

#include "../algebra/side.hpp"
#include "../algebra/variable.hpp"
#include "../record/name.hpp"

namespace upd {

enum class vartype {
  value,
  length,
  count,
  code,
};

template<std::size_t N>
struct varname {
  vartype type;
  name<N> id;
};

template<name Identifier>
constexpr auto value_of = upd::algebra::side{
    upd::algebra::variable<varname{vartype::value, Identifier}>{}};

template<name Identifier>
constexpr auto length_of = upd::algebra::side{
    upd::algebra::variable<varname{vartype::length, Identifier}>{}};

template<name Identifier>
constexpr auto count_of = upd::algebra::side{
    upd::algebra::variable<varname{vartype::count, Identifier}>{}};

template<name Identifier>
constexpr auto code_of = upd::algebra::side{
    upd::algebra::variable<varname{vartype::code, Identifier}>{}};

} // namespace upd

template<std::size_t N>
struct std::formatter<upd::varname<N>> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(upd::varname<N> vn, std::format_context &ctx) {
    auto it = ctx.out();

    switch (vn.type) {
    case upd::vartype::value:
      it = std::format_to(it, "[value of ");
      break;
    case upd::vartype::length:
      it = std::format_to(it, "[length of ");
      break;
    case upd::vartype::count:
      it = std::format_to(it, "[count of ");
      break;
    case upd::vartype::code:
      it = std::format_to(it, "[code of ");
      break;
    default:
      it = std::format_to(it, "[??? of ");
      break;
    };
    it = std::format_to(it, "{}]", vn.id);

    ctx.advance_to(it);
    return it;
  }
};
