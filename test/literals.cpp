#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <upd/detail/always_false.hpp>
#include <upd/literals.hpp>

TEST_CASE("Create `std::integral_constant` literals from") {
  using namespace upd::literals;

  REQUIRE(15_ic == std::integral_constant<unsigned, 15>{});
  REQUIRE(015_ic == std::integral_constant<unsigned, 015>{});
  REQUIRE(0xab_ic == std::integral_constant<unsigned, 0xab>{});
  REQUIRE(0Xab_ic == std::integral_constant<unsigned, 0Xab>{});
  REQUIRE(0b111_ic == std::integral_constant<unsigned, 0b111>{});
  REQUIRE(-15_ic == std::integral_constant<int, -15>{});
  REQUIRE(-015_ic == std::integral_constant<int, -015>{});
  REQUIRE(-0xab_ic == std::integral_constant<int, -0xab>{});
  REQUIRE(-0Xab_ic == std::integral_constant<int, -0Xab>{});
  REQUIRE(-0b111_ic == std::integral_constant<int, -0b111>{});
}
