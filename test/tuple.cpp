#include <tuple>

#include <catch2/catch_test_macros.hpp>
#include <upd/tuple_v2.hpp>

TEST_CASE("Tuple views", "[tuple_view]") {
  namespace updv = upd::tuple_views;

  upd::regular_tuple auto t = std::tuple{int{4}, char{8}, long{67}};

  SECTION("Transform element of a tuple") {
    upd::tuple_view auto view = t | updv::transform([](auto x) { return x + 1; });
    REQUIRE(get<0>(view) == 5);
    REQUIRE(get<1>(view) == 9);
    REQUIRE(get<2>(view) == 68);
  }
}
