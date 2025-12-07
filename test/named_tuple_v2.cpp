#include <catch2/catch_test_macros.hpp>
#include <upd/constexpr.hpp>
#include <upd/named_value.hpp>

TEST_CASE("Testing element access features", "[named_tuple]") {
  using namespace upd::literals;

  upd::record_like auto nt = upd::named_tuple{"a"_kw = 1, "b"_kw = 2, "c"_kw = 3};

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

TEST_CASE("Testing element access features tagged", "[tagged_tuple]") {
  using namespace upd::literals;

  upd::tuple_like2 auto nt = upd::tagged_tuple{"a"_kw = 1, "b"_kw = 2, "c"_kw = 3};

  SECTION("Access tuple through tags") {
    REQUIRE(nt["a"_kw] == 1);
    REQUIRE(nt["b"_kw] == 2);
    REQUIRE(nt["c"_kw] == 3);
  }

  SECTION("Access tuple through indices") {
    REQUIRE(nt[upd::expr<0uz>] == 1);
    REQUIRE(nt[upd::expr<1uz>] == 2);
    REQUIRE(nt[upd::expr<2uz>] == 3);
  }
}
