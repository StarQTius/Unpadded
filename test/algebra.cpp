#include <catch2/catch_test_macros.hpp>
#include <upd/algebra.hpp>

TEST_CASE("Independent equation side", "[algebra][side]") {
  using namespace upd::algebra::literals;

  SECTION("Solve integer linear expression with divisible operands") {
    auto result = ("x"_var / 3 + 2).calculate(upd::algebra::let{"x"_var, 6});
    REQUIRE(result == 4);
  }

  SECTION("Solve integer linear expression with undivisible operands") {
    auto result = ("x"_var / 3 + 2).calculate(upd::algebra::let{"x"_var, 7});
    REQUIRE(result == 4);
  }
}
