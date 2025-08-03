#include <array>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <fakeit.hpp>
#include <upd/detail/always_false.hpp>
#include <upd/named_value.hpp>

TEST_CASE("Testing element access features", "[named_tuple]") {
  using namespace upd::literals;

  auto nt = upd::named_tuple{"a"_kw = 1, "b"_kw = 2, "c"_kw = 3};

  SECTION("Access tuple through tags") {
    REQUIRE(nt["a"_kw] == 1);
    REQUIRE(nt["b"_kw] == 2);
    REQUIRE(nt["c"_kw] == 3);
  }

  SECTION("Access tuple through indices") {
    REQUIRE(nt[upd::expr<0uz>].value() == 1);
    REQUIRE(nt[upd::expr<1uz>].value() == 2);
    REQUIRE(nt[upd::expr<2uz>].value() == 3);
  }
}
