#pragma once

#include <catch2/catch_test_macros.hpp>

template<typename C>
auto iterate(C &c) noexcept {
  using std::begin;
  using std::end;

  auto retval = [&, it = begin(c)](auto...) mutable {
    REQUIRE(it != end(c));
    return *it++;
  };

  return retval;
}
