#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <upd/algebra.hpp>
#include <upd/record.hpp>
#include <upd/tuple_v2.hpp>

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
    REQUIRE(upd::algebra::depends_on_v<decltype(s.expr), upd::name{"x"}>);
    REQUIRE(upd::algebra::depends_on_v<decltype(s.expr), upd::name{"y"}>);
    REQUIRE(upd::algebra::depends_on_v<decltype(s.expr), upd::name{"z"}>);
    REQUIRE(!upd::algebra::depends_on_v<decltype(s.expr), upd::name{"a"}>);
    REQUIRE(upd::algebra::depends_on_v<decltype(s.expr.lhs), upd::name{"x"}>);
    REQUIRE(!upd::algebra::depends_on_v<decltype(s.expr.lhs), upd::name{"y"}>);
    REQUIRE(!upd::algebra::depends_on_v<decltype(s.expr.lhs), upd::name{"z"}>);
    REQUIRE(!upd::algebra::depends_on_v<decltype(s.expr.rhs), upd::name{"x"}>);
    REQUIRE(upd::algebra::depends_on_v<decltype(s.expr.rhs), upd::name{"y"}>);
    REQUIRE(upd::algebra::depends_on_v<decltype(s.expr.rhs), upd::name{"z"}>);
  }
}

TEST_CASE("Identity", "[algebra][equation]") {
  using namespace upd::algebra::literals;

  SECTION("Make an `upd::let` object from an identity") {
    auto lt = upd::algebra::let{"x"_var = 6};
    REQUIRE(lt.var == upd::algebra::variable<upd::name{"x"}>{});
    REQUIRE(lt.val == 6);
  }

  SECTION("Simplify a one-variable equation into an identity") {
    auto eq = ("x"_var * 5 + 3 = 18);
    auto id = eq.simplify();
    REQUIRE(id == ("x"_var = 3));
  }
}

TEST_CASE("Two-variable equation", "[algebra][equation]") {
  using namespace upd::algebra::literals;

  SECTION("Substitute an integer for a variable") {
    auto redeq = ("y"_var = "x"_var / 2 + 3).substitute("x"_var = 8);
    REQUIRE(redeq.lhs == upd::algebra::variable<upd::name{"y"}>{});
    REQUIRE(redeq.rhs == 7);
  }

  SECTION("Isolate on variable on the a side") {
    auto eq1 = ("y"_var = "x"_var / 3 + 4).isolate("x"_var);
    REQUIRE(eq1.lhs == "x"_var.expr);
    REQUIRE(eq1.rhs == (("y"_var - 4) * 3).expr);

    auto eq2 = ("y"_var = "x"_var * 7 - 6).isolate("x"_var);
    REQUIRE(eq2.lhs == "x"_var.expr);
    REQUIRE(eq2.rhs == (("y"_var + 6) / 7).expr);

    auto eq3 = ("y"_var = 4 + 3 / "x"_var).isolate("x"_var);
    REQUIRE(eq3.lhs == "x"_var.expr);
    REQUIRE(eq3.rhs == (3 / ("y"_var - 4)).expr);

    auto eq4 = ("y"_var = 6 - 7 * "x"_var).isolate("x"_var);
    REQUIRE(eq4.lhs == "x"_var.expr);
    REQUIRE(eq4.rhs == ((6 - "y"_var) / 7).expr);
  }
}

TEST_CASE("System", "[algebra][system]") {
  using namespace upd::algebra::literals;
  namespace updv = upd::tuple_views;

  auto sys = std::tuple{"y"_var = 2 * "x"_var + 3, "z"_var = 4 * "x"_var - 2, "x"_var = 3 * "a"_var - 11};

  SECTION("Substitute a variable") {
    auto y = solve_for("y"_var, updv::concat(sys, std::tuple{"x"_var = 4}));
    REQUIRE(y == 11);

    auto z = solve_for("z"_var, updv::concat(sys, std::tuple{"x"_var = 4}));
    REQUIRE(z == 14);

    auto a = solve_for("a"_var, updv::concat(sys, std::tuple{"x"_var = 4}));
    REQUIRE(a == 5);
  }
}
