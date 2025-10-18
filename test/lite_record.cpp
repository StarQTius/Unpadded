#include <concepts>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <upd/constexpr.hpp>
#include <upd/named_value.hpp>
#include <upd/record.hpp>
#include <upd/type_traits.hpp>

TEST_CASE("Lite record basic functionalities", "[lite_record]") {
  upd::record_like auto rec = upd::lite_record{upd::lite_record_node{upd::expr<upd::name{"a"}>, int{4}},
                                               upd::lite_record_node{upd::expr<upd::name{"b"}>, char{8}},
                                               upd::lite_record_node{upd::expr<upd::name{"c"}>, false}};

  SECTION("Get elements from their tag") {
    REQUIRE(rec.get_by_tag(upd::expr<upd::name{"a"}>) == 4);
    REQUIRE(rec.get_by_tag(upd::expr<upd::name{"b"}>) == 8);
    REQUIRE(!rec.get_by_tag(upd::expr<upd::name{"c"}>));
  }

  SECTION("Get element types from their tag") {
    REQUIRE(rec.get_type_by_tag(upd::expr<upd::name{"a"}>) == upd::typebox<int>{});
    REQUIRE(rec.get_type_by_tag(upd::expr<upd::name{"b"}>) == upd::typebox<char>{});
    REQUIRE(rec.get_type_by_tag(upd::expr<upd::name{"c"}>) == upd::typebox<bool>{});
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

  SECTION("Access elements when record is qualified") {
    auto &&lv = rec.get_by_tag(upd::expr<upd::name{"a"}>);
    auto &&cst_lv = std::as_const(rec).get_by_tag(upd::expr<upd::name{"a"}>);
    auto &&rv = std::move(rec).get_by_tag(upd::expr<upd::name{"a"}>);
    auto &&cst_rv = std::move(std::as_const(rec)).get_by_tag(upd::expr<upd::name{"a"}>);

    REQUIRE(std::same_as<decltype(lv), int &>);
    REQUIRE(std::same_as<decltype(cst_lv), const int &>);
    REQUIRE(std::same_as<decltype(rv), int &&>);
    REQUIRE(std::same_as<decltype(cst_rv), const int &&>);
  }
}

TEST_CASE("Babelian lite record", "[universal_record]") {
  upd::record_like auto rec = upd::universal_record{42};

  SECTION("Check if record has a tag") {
    REQUIRE(has_tag<upd::name{"a"}>(rec));
    REQUIRE(has_tag<upd::name{"b"}>(rec));
    REQUIRE(has_tag<upd::name{"c"}>(rec));
    REQUIRE(has_tag<upd::name{"d"}>(rec));
    REQUIRE(has_tag<upd::name{"e"}>(rec));
  }

  SECTION("Check if record has an element of given type") {
    REQUIRE(has_type<int>(rec));
    REQUIRE(!has_type<char>(rec));
    REQUIRE(!has_type<bool>(rec));
    REQUIRE(!has_type<short>(rec));
    REQUIRE(!has_type<long>(rec));
  }

  SECTION("Access elements when record is qualified") {
    auto &&lv = get<upd::name{"a"}>(rec);
    auto &&cst_lv = get<upd::name{"a"}>(std::as_const(rec));
    auto &&rv = get<upd::name{"a"}>(std::move(rec));
    auto &&cst_rv = get<upd::name{"a"}>(std::move(std::as_const(rec)));

    REQUIRE(std::same_as<decltype(lv), const int &>);
    REQUIRE(std::same_as<decltype(cst_lv), const int &>);
    REQUIRE(std::same_as<decltype(rv), const int &&>);
    REQUIRE(std::same_as<decltype(cst_rv), const int &&>);
  }
}
