#include <catch2/catch_test_macros.hpp>
#include <upd/constexpr.hpp>
#include <upd/lite_record.hpp>
#include <upd/named_value.hpp>
#include <upd/type_traits.hpp>

TEST_CASE("Lite record", "[lite_record]") {
  upd::lite_record rec{upd::lite_record_node{upd::expr<upd::name{"a"}>, int{4}},
                       upd::lite_record_node{upd::expr<upd::name{"b"}>, char{8}},
                       upd::lite_record_node{upd::expr<upd::name{"c"}>, false}};

  SECTION("Get elements from their tag") {
    REQUIRE(rec.get_by_tag(upd::expr<upd::name{"a"}>) == 4);
    REQUIRE(rec.get_by_tag(upd::expr<upd::name{"b"}>) == 8);
    REQUIRE(!rec.get_by_tag(upd::expr<upd::name{"c"}>));
  }

  SECTION("Find tags from their element type") {
    REQUIRE(rec.find_by_type(upd::typebox<int>{}) == upd::expr<upd::name{"a"}>);
    REQUIRE(rec.find_by_type(upd::typebox<char>{}) == upd::expr<upd::name{"b"}>);
    REQUIRE(rec.find_by_type(upd::typebox<bool>{}) == upd::expr<upd::name{"c"}>);
  }

  SECTION("Check if record has a tag") {
    REQUIRE(rec.has_tag(upd::expr<upd::name{"a"}>));
    REQUIRE(rec.has_tag(upd::expr<upd::name{"b"}>));
    REQUIRE(rec.has_tag(upd::expr<upd::name{"c"}>));
    REQUIRE(!rec.has_tag(upd::expr<upd::name{"d"}>));
    REQUIRE(!rec.has_tag(upd::expr<upd::name{"e"}>));
  }

  SECTION("Check if record has an element of given type") {
    REQUIRE(rec.has_type(upd::typebox<int>{}));
    REQUIRE(rec.has_type(upd::typebox<char>{}));
    REQUIRE(rec.has_type(upd::typebox<bool>{}));
    REQUIRE(!rec.has_type(upd::typebox<short>{}));
    REQUIRE(!rec.has_type(upd::typebox<long>{}));
  }
}
