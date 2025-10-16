#include <catch2/catch_test_macros.hpp>
#include <upd/algebra.hpp>
#include <upd/named_value.hpp>

TEST_CASE("Independent equation side", "[algebra][side]") {
  using namespace upd::algebra::literals;

  SECTION("Add a variable and an integer") {
    auto result = ("x"_var + 5).calculate(upd::algebra::let{"x"_var, 1});
    REQUIRE(result == 6);
  }

  SECTION("Add an integer and a variable") {
    auto result = (2 + "x"_var).calculate(upd::algebra::let{"x"_var, 6});
    REQUIRE(result == 8);
  }

  SECTION("Substract a variable from an integer") {
    auto result = (5 - "x"_var).calculate(upd::algebra::let{"x"_var, 1});
    REQUIRE(result == 4);
  }

  SECTION("Substract an integer from variable") {
    auto result = ("x"_var - 4).calculate(upd::algebra::let{"x"_var, 1});
    REQUIRE(result == -3);
  }

  SECTION("Multiply a variable with an integer") {
    auto result = ("x"_var * 5).calculate(upd::algebra::let{"x"_var, 2});
    REQUIRE(result == 10);
  }

  SECTION("Multiply an integer with a variable") {
    auto result = (3 * "x"_var).calculate(upd::algebra::let{"x"_var, 6});
    REQUIRE(result == 18);
  }

  SECTION("Divide an integer by a variable") {
    auto result = (9 / "x"_var).calculate(upd::algebra::let{"x"_var, 3});
    REQUIRE(result == 3);
  }

  SECTION("Divide a variable by an integer") {
    auto result = ("x"_var / 4).calculate(upd::algebra::let{"x"_var, 20});
    REQUIRE(result == 5);
  }

  SECTION("Solve integer linear expression with divisible operands") {
    auto result = ("x"_var / 3 + 2).calculate(upd::algebra::let{"x"_var, 6});
    REQUIRE(result == 4);
  }

  SECTION("Solve integer linear expression with undivisible operands") {
    auto result = ("x"_var / 3 + 2).calculate(upd::algebra::let{"x"_var, 7});
    REQUIRE(result == 4);
  }

  SECTION("Solve integer linear expression with several variables") {
    auto result = ("x"_var / 2 + "y"_var / 3 + 7).calculate("x"_var = 6, "y"_var = 9);
    REQUIRE(result == 13);
  }

  SECTION("Check whether an equation side depends on specific variables") {
    auto s = "x"_var / 4 + ("y"_var / 2 + "z"_var);
    REQUIRE(depends_on<upd::name{"x"}>(s.expr));
    REQUIRE(depends_on<upd::name{"y"}>(s.expr));
    REQUIRE(depends_on<upd::name{"z"}>(s.expr));
    REQUIRE(!depends_on<upd::name{"a"}>(s.expr));
    REQUIRE(depends_on<upd::name{"x"}>(s.expr.lhs));
    REQUIRE(!depends_on<upd::name{"y"}>(s.expr.lhs));
    REQUIRE(!depends_on<upd::name{"z"}>(s.expr.lhs));
    REQUIRE(!depends_on<upd::name{"x"}>(s.expr.rhs));
    REQUIRE(depends_on<upd::name{"y"}>(s.expr.rhs));
    REQUIRE(depends_on<upd::name{"z"}>(s.expr.rhs));
  }
}

TEST_CASE("Identity", "[algebra][equation]") {
  using namespace upd::algebra::literals;

  SECTION("Make an `upd::let` object from an identity") {
    auto lt = upd::algebra::let{"x"_var = 6};
    REQUIRE(lt.var == upd::algebra::variable<upd::name{"x"}>{});
    REQUIRE(lt.val == 6);
  }
}

TEST_CASE("Two-variable equation", "[algebra][equation]") {
  using namespace upd::algebra::literals;

  SECTION("Substitute an integer for a variable") {
    auto redeq = ("y"_var = "x"_var / 2 + 3).substitute("x"_var = 8);
    REQUIRE(redeq.lhs == upd::algebra::variable<upd::name{"y"}>{});
    REQUIRE(redeq.rhs == 7);
  }
}
