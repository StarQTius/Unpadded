#include <cstddef>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>
#include <upd/detail/integral_constant.hpp>
#include <upd/detail/range.hpp>
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

  REQUIRE(15_i == std::integral_constant<std::size_t, 15>{});
  REQUIRE(015_i == std::integral_constant<std::size_t, 015>{});
  REQUIRE(0xab_i == std::integral_constant<std::size_t, 0xab>{});
  REQUIRE(0Xab_i == std::integral_constant<std::size_t, 0Xab>{});
  REQUIRE(0b111_i == std::integral_constant<std::size_t, 0b111>{});
}
