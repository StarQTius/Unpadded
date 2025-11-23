#pragma once

#include <cassert>
#include <regex>
#include <source_location>
#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#define REQUIRE_SAME(LHS, RHS)                                                                                         \
  do {                                                                                                                 \
    if constexpr (!std::same_as<decltype(LHS), decltype(RHS)>) {                                                       \
      FAIL(std::format("{} != {}", identifier_of<decltype(LHS)>(), identifier_of<decltype(RHS)>()));                   \
    }                                                                                                                  \
    REQUIRE(address_of(LHS) == address_of(RHS));                                                                       \
  } while (false);

template<typename T>
inline auto address_of(T &&x) noexcept {
  return std::addressof(x);
}

template<typename T>
inline auto identifier_of() -> std::string {
  const auto *fname = std::source_location::current().function_name();
  auto pattern = std::regex{"std::string identifier_of\\(\\) \\[T = (.*)\\]"};
  auto match = std::cmatch{};
  auto success = std::regex_match(fname, match, pattern);
  assert(success);
  return match[1];
}
