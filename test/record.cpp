#include <concepts>
#include <string>
#include <string_view>
#include <utility>

#include "utility.hpp"
#include <catch2/catch_test_macros.hpp>
#include <upd/constexpr.hpp>
#include <upd/equivalent_to.hpp>
#include <upd/named_value.hpp>
#include <upd/record.hpp>
#include <upd/tuple_v2.hpp>
#include <upd/type_traits.hpp>
#include <upd/upd.hpp>

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

    REQUIRE(upd::has_tag<upd::name{"a"}>(rec));
    REQUIRE(upd::has_tag<upd::name{"b"}>(rec));
    REQUIRE(upd::has_tag<upd::name{"c"}>(rec));
    REQUIRE(!upd::has_tag<upd::name{"d"}>(rec));
    REQUIRE(!upd::has_tag<upd::name{"e"}>(rec));
  }

  SECTION("Check if record has an element of given type") {
    REQUIRE(rec.has_type(upd::typebox<int>{}));
    REQUIRE(rec.has_type(upd::typebox<char>{}));
    REQUIRE(rec.has_type(upd::typebox<bool>{}));
    REQUIRE(!rec.has_type(upd::typebox<short>{}));
    REQUIRE(!rec.has_type(upd::typebox<long>{}));

    REQUIRE(upd::has_type<int>(rec));
    REQUIRE(upd::has_type<char>(rec));
    REQUIRE(upd::has_type<bool>(rec));
    REQUIRE(!upd::has_type<short>(rec));
    REQUIRE(!upd::has_type<long>(rec));
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
    REQUIRE(upd::has_tag<upd::name{"a"}>(rec));
    REQUIRE(upd::has_tag<upd::name{"b"}>(rec));
    REQUIRE(upd::has_tag<upd::name{"c"}>(rec));
    REQUIRE(upd::has_tag<upd::name{"d"}>(rec));
    REQUIRE(upd::has_tag<upd::name{"e"}>(rec));
  }

  SECTION("Check if record has an element of given type") {
    REQUIRE(upd::has_type<int>(rec));
    REQUIRE(!upd::has_type<char>(rec));
    REQUIRE(!upd::has_type<bool>(rec));
    REQUIRE(!upd::has_type<short>(rec));
    REQUIRE(!upd::has_type<long>(rec));
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

TEST_CASE("Record basic functionalities", "[record]") {
  using namespace upd::literals;

  upd::record_like auto rec = upd::record{"a"_kw2 = int{4}, "b"_kw2 = char{8}, "c"_kw2 = false};

  SECTION("Access elements when record is qualified") {
    auto &&lv = rec["a"_kw2];
    auto &&cst_lv = std::as_const(rec)["a"_kw2];
    auto &&rv = std::move(rec)["a"_kw2];
    auto &&cst_rv = std::move(std::as_const(rec))["a"_kw2];

    REQUIRE(std::same_as<decltype(lv), int &>);
    REQUIRE(std::same_as<decltype(cst_lv), const int &>);
    REQUIRE(std::same_as<decltype(rv), int &&>);
    REQUIRE(std::same_as<decltype(cst_rv), const int &&>);
  }

  SECTION("Check if record has a tag") {
    REQUIRE(upd::has_tag<upd::name{"a"}>(rec));
    REQUIRE(upd::has_tag<upd::name{"b"}>(rec));
    REQUIRE(upd::has_tag<upd::name{"c"}>(rec));
    REQUIRE(!upd::has_tag<upd::name{"d"}>(rec));
    REQUIRE(!upd::has_tag<upd::name{"e"}>(rec));
  }
}

TEST_CASE("Record views", "[record_view]") {
  namespace updv = upd::record_views;
  using namespace upd::literals;

  upd::record_like auto rec = upd::record{"a"_kw2 = int{4}, "b"_kw2 = char{8}, "c"_kw2 = long{67}};

  SECTION("Transform element of a record") {
    upd::record_view auto view = rec | updv::transform([](auto, auto x) { return x + 1; });
    REQUIRE(get<upd::name{"a"}>(view) == 5);
    REQUIRE(get<upd::name{"b"}>(view) == 9);
    REQUIRE(get<upd::name{"c"}>(view) == 68);
  }

  SECTION("Clean elements from a record") {
    upd::record_like auto view = rec | updv::clean<char &>;
    REQUIRE(has_tag<upd::name{"a"}>(view));
    REQUIRE(!has_tag<upd::name{"b"}>(view));
    REQUIRE(has_tag<upd::name{"c"}>(view));
    REQUIRE(&get<upd::name{"a"}>(view) == &get<upd::name{"a"}>(rec));
    REQUIRE(&get<upd::name{"c"}>(view) == &get<upd::name{"c"}>(rec));
  }

  SECTION("Transform element by substituting references for them") {
    auto x = 0;
    upd::record_view auto view = rec | updv::transform([&](auto, auto) -> auto && { return x; });

    REQUIRE(&get<upd::name{"a"}>(view) == &x);
    REQUIRE(&get<upd::name{"b"}>(view) == &x);
    REQUIRE(&get<upd::name{"c"}>(view) == &x);
  }

  SECTION("Transform element according to their tag") {
    using namespace std::literals;

    upd::record_view auto view = rec | updv::transform([&](auto k, auto) { return k.value.string; });

    REQUIRE(get<upd::name{"a"}>(view) == "a"sv);
    REQUIRE(get<upd::name{"b"}>(view) == "b"sv);
    REQUIRE(get<upd::name{"c"}>(view) == "c"sv);
  }

  SECTION("Filter element from a record") {
    upd::record_view auto view = rec | updv::filter([]<auto Id, typename T>(upd::auto_constant<Id>, upd::typebox<T>) {
                                   return !upd::equivalent_to<Id, upd::name{"b"}> && !std::same_as<T, int &>;
                                 });

    REQUIRE(!has_tag<upd::name{"a"}>(view));
    REQUIRE(!has_tag<upd::name{"b"}>(view));
    REQUIRE(has_tag<upd::name{"c"}>(view));
    REQUIRE(&get<upd::name{"c"}>(view) == &get<upd::name{"c"}>(rec));
  }

  SECTION("Join entries of a record") {
    upd::nested_record auto nested_rec = upd::record{"a"_kw2 =
                                                         upd::record{
                                                             "1"_kw2 = 1,
                                                             "2"_kw2 = 2,
                                                         },
                                                     "b"_kw2 = upd::record{
                                                         "3"_kw2 = 3,
                                                         "4"_kw2 = 4,
                                                     }};

    auto view = nested_rec | updv::join([](auto pk, auto k) { return upd::name{{pk.string[0], k.string[0], 0}}; });

    REQUIRE(get<upd::name{"a1"}>(view) == 1);
    REQUIRE(get<upd::name{"a2"}>(view) == 2);
    REQUIRE(get<upd::name{"b3"}>(view) == 3);
    REQUIRE(get<upd::name{"b4"}>(view) == 4);
  }

  SECTION("Enumerate a record") {
    auto view = rec | updv::enumerate;

    REQUIRE(get<upd::name{"a"}>(view) == std::pair{0, 4});
    REQUIRE(get<upd::name{"b"}>(view) == std::pair{1, 8});
    REQUIRE(get<upd::name{"c"}>(view) == std::pair{2, 67});
  }

  SECTION("Reverse a record") {
    auto view = rec | updv::reverse;

    REQUIRE(&get<upd::name{"a"}>(view) == &get<upd::name{"a"}>(rec));
    REQUIRE(&get<upd::name{"b"}>(view) == &get<upd::name{"b"}>(rec));
    REQUIRE(&get<upd::name{"c"}>(view) == &get<upd::name{"c"}>(rec));
    REQUIRE(&upd::get_ith<0>(view) == &upd::get_ith<2>(rec));
    REQUIRE(&upd::get_ith<1>(view) == &upd::get_ith<1>(rec));
    REQUIRE(&upd::get_ith<2>(view) == &upd::get_ith<0>(rec));
  }

  SECTION("Zip two records together") {
    upd::record_like auto rec_ = upd::record{"1"_kw2 = bool{true}, "2"_kw2 = unsigned{9}, "3"_kw2 = bool{false}};

    auto view =
        updv::zip(rec, rec_, [](auto tag1, auto tag2) { return upd::name{{tag1.string[0], tag2.string[0], 0}}; });

    REQUIRE(&upd::get<upd::name{"a1"}>(view).first == &upd::get<upd::name{"a"}>(rec));
    REQUIRE(&upd::get<upd::name{"a1"}>(view).second == &upd::get<upd::name{"1"}>(rec_));
    REQUIRE(&upd::get<upd::name{"b2"}>(view).first == &upd::get<upd::name{"b"}>(rec));
    REQUIRE(&upd::get<upd::name{"b2"}>(view).second == &upd::get<upd::name{"2"}>(rec_));
    REQUIRE(&upd::get<upd::name{"c3"}>(view).first == &upd::get<upd::name{"c"}>(rec));
    REQUIRE(&upd::get<upd::name{"c3"}>(view).second == &upd::get<upd::name{"3"}>(rec_));
  }

  SECTION("Collect a view into a regular record") {
    upd::regular_record auto regec = rec | updv::transform([](auto, auto v) { return v + 1; }) |
                                     updv::filter([](auto k, auto) { return k != "b"; }) | updv::to<upd::record>;

    REQUIRE(regec["a"_kw2] == 5);
    REQUIRE(!upd::has_tag<upd::name{"b"}>(regec));
    REQUIRE(regec["c"_kw2] == 68);
  }

  SECTION("Instantiate a record template") {
    using record_type =
        decltype(rec | updv::transform([](auto, auto &x) { return &x; }) | updv::instantiate<upd::record>);
    REQUIRE(std::same_as<record_type,
                         upd::record<upd::entry<upd::name{"a"}, int *>,
                                     upd::entry<upd::name{"b"}, char *>,
                                     upd::entry<upd::name{"c"}, long *>>>);
  }

  SECTION("View a record as a tuple of entries") {
    upd::tuple_view auto view = rec | updv::as_tuple;
    REQUIRE(get<0>(view).identifier == upd::name{"a"});
    REQUIRE(&get<0>(view).value == &get<upd::name{"a"}>(rec));
    REQUIRE(get<1>(view).identifier == upd::name{"b"});
    REQUIRE(&get<1>(view).value == &get<upd::name{"b"}>(rec));
    REQUIRE(get<2>(view).identifier == upd::name{"c"});
    REQUIRE(&get<2>(view).value == &get<upd::name{"c"}>(rec));
  }
}

TEST_CASE("Algorithms on records", "[record_algorithm]") {
  namespace updv = upd::record_views;
  using namespace upd::literals;

  upd::record_like auto rec = upd::record{"a"_kw2 = int{4}, "b"_kw2 = char{8}, "c"_kw2 = long{67}};

  SECTION("Left-fold record content") {
    using namespace std::literals;

    auto res = updv::fold_left(rec, std::pair{""s, 0}, [](auto acc, auto k, auto v) {
      return std::pair{acc.first + k.value.string, acc.second + v};
    });

    REQUIRE(res == std::pair{"abc", 79});
  }

  SECTION("Right-fold record content") {
    using namespace std::literals;

    auto res = updv::fold_right(rec, std::pair{""s, 0}, [](auto k, auto v, auto acc) {
      return std::pair{acc.first + k.value.string, acc.second + v};
    });

    REQUIRE(res == std::pair{"cba", 79});
  }

  SECTION("Find the first value matching a predicate") {
    auto i = updv::find_if(rec, []<typename T>(auto, upd::typebox<T>) { return std::same_as<T, long &>; });

    REQUIRE(i == 2);
  }

  SECTION("Find by a predicate that matches no value") {
    auto i = updv::find_if(rec, []<typename T>(auto, upd::typebox<T>) { return std::same_as<T, void>; });

    REQUIRE(i == 3);
  }
}

TEST_CASE("Record view handling references", "[record_view]") {
  namespace updv = upd::record_views;
  using namespace upd::literals;

  auto lv = 12;
  auto xv = 23;
  upd::record_like auto rec = upd::record{
      upd::entry<upd::name{"l"}, int &>{lv}, upd::entry<upd::name{"x"}, int &&>{std::move(xv)}, "pr"_kw2 = 34};

  SECTION("Pass references through transform") {
    upd::record_view auto lview = rec | updv::transform([](auto, auto &&v) -> auto && { return UPD_FWD(v); });

    REQUIRE_SAME(get<upd::name{"l"}>(lview), (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(lview), (xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(lview), rec["pr"_kw2]);

    upd::record_view auto rview =
        std::move(rec) | updv::transform([](auto, auto &&v) -> auto && { return UPD_FWD(v); });

    REQUIRE_SAME(get<upd::name{"l"}>(rview), (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(rview), std::move(xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(rview), std::move(rec)["pr"_kw2]);
  }

  SECTION("Pass references through clean") {
    upd::record_view auto lview = rec | updv::clean<void>;

    REQUIRE_SAME(get<upd::name{"l"}>(lview), (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(lview), (xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(lview), rec["pr"_kw2]);

    upd::record_view auto rview = std::move(rec) | updv::clean<void>;

    REQUIRE_SAME(get<upd::name{"l"}>(rview), (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(rview), std::move(xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(rview), std::move(rec)["pr"_kw2]);
  }

  SECTION("Pass references through filter") {
    upd::record_view auto lview = rec | updv::filter([](auto, auto) { return true; });

    REQUIRE_SAME(get<upd::name{"l"}>(lview), (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(lview), (xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(lview), rec["pr"_kw2]);

    upd::record_view auto rview = std::move(rec) | updv::filter([](auto, auto) { return true; });

    REQUIRE_SAME(get<upd::name{"l"}>(rview), (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(rview), std::move(xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(rview), std::move(rec)["pr"_kw2]);
  }

  SECTION("Pass references through enumerate") {
    upd::record_view auto lview = rec | updv::enumerate;

    REQUIRE_SAME(get<upd::name{"l"}>(lview).second, (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(lview).second, (xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(lview).second, rec["pr"_kw2]);

    upd::record_view auto rview = std::move(rec) | updv::enumerate;

    REQUIRE_SAME(get<upd::name{"l"}>(rview).second, (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(rview).second, std::move(xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(rview).second, std::move(rec)["pr"_kw2]);
  }

  SECTION("Pass references through join") {
    upd::nested_record auto nested_rec = upd::record{"a"_kw2 = std::move(rec),
                                                     upd::entry<upd::name{"b"}, decltype(rec) &>{rec},
                                                     upd::entry<upd::name{"c"}, decltype(rec) &&>{std::move(rec)}};

    auto lview = nested_rec | updv::join([](auto pk, auto k) { return upd::name{{pk.string[0], k.string[0], 0}}; });

    REQUIRE_SAME(get<upd::name{"al"}>(lview), (lv));
    REQUIRE_SAME(get<upd::name{"ax"}>(lview), (xv));
    REQUIRE_SAME(get<upd::name{"ap"}>(lview), nested_rec["a"_kw2]["pr"_kw2]);
    REQUIRE_SAME(get<upd::name{"bl"}>(lview), (lv));
    REQUIRE_SAME(get<upd::name{"bx"}>(lview), (xv));
    REQUIRE_SAME(get<upd::name{"bp"}>(lview), rec["pr"_kw2]);
    REQUIRE_SAME(get<upd::name{"cl"}>(lview), (lv));
    REQUIRE_SAME(get<upd::name{"cx"}>(lview), (xv));
    REQUIRE_SAME(get<upd::name{"cp"}>(lview), rec["pr"_kw2]);

    auto rview =
        std::move(nested_rec) | updv::join([](auto pk, auto k) { return upd::name{{pk.string[0], k.string[0], 0}}; });

    REQUIRE_SAME(get<upd::name{"al"}>(rview), (lv));
    REQUIRE_SAME(get<upd::name{"ax"}>(rview), std::move(xv));
    REQUIRE_SAME(get<upd::name{"ap"}>(rview), std::move(nested_rec)["a"_kw2]["pr"_kw2]);
    REQUIRE_SAME(get<upd::name{"bl"}>(rview), (lv));
    REQUIRE_SAME(get<upd::name{"bx"}>(rview), (xv));
    REQUIRE_SAME(get<upd::name{"bp"}>(rview), rec["pr"_kw2]);
    REQUIRE_SAME(get<upd::name{"cl"}>(rview), (lv));
    REQUIRE_SAME(get<upd::name{"cx"}>(rview), std::move(xv));
    REQUIRE_SAME(get<upd::name{"cp"}>(rview), std::move(rec)["pr"_kw2]);
  }

  SECTION("Pass references through reverse") {
    auto lview = rec | updv::reverse;

    REQUIRE_SAME(get<upd::name{"l"}>(lview), (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(lview), (xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(lview), rec["pr"_kw2]);

    auto rview = std::move(rec) | updv::reverse;

    REQUIRE_SAME(get<upd::name{"l"}>(rview), (lv));
    REQUIRE_SAME(get<upd::name{"x"}>(rview), std::move(xv));
    REQUIRE_SAME(get<upd::name{"pr"}>(rview), std::move(rec)["pr"_kw2]);
  }

  SECTION("Pass references through zip") {
    auto view = updv::zip(
        rec, std::move(rec), [](auto tag1, auto tag2) { return upd::name{{tag1.string[0], tag2.string[0], 0}}; });

    REQUIRE_SAME(get<upd::name{"ll"}>(view).first, (lv));
    REQUIRE_SAME(get<upd::name{"ll"}>(view).second, (lv));
    REQUIRE_SAME(get<upd::name{"xx"}>(view).first, (xv));
    REQUIRE_SAME(get<upd::name{"xx"}>(view).second, std::move(xv));
    REQUIRE_SAME(get<upd::name{"pp"}>(view).first, rec["pr"_kw2]);
    REQUIRE_SAME(get<upd::name{"pp"}>(view).second, std::move(rec)["pr"_kw2]);
  }
}
