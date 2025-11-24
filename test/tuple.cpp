#include <concepts>
#include <tuple>
#include <utility>

#include "utility.hpp"
#include <catch2/catch_test_macros.hpp>
#include <upd/constexpr.hpp>
#include <upd/tuple_v2.hpp>
#include <upd/type_traits.hpp>
#include <upd/upd.hpp>

TEST_CASE("Tuple views", "[tuple_view]") {
  namespace updv = upd::tuple_views;

  upd::regular_tuple auto t = std::tuple{int{4}, char{8}, long{67}};

  SECTION("Transform element of a tuple") {
    upd::tuple_view auto view = t | updv::transform([](auto x) { return x + 1; });
    REQUIRE(get<0>(view) == 5);
    REQUIRE(get<1>(view) == 9);
    REQUIRE(get<2>(view) == 68);
  }

  SECTION("Clean elements of a tuple") {
    upd::tuple_view auto view = t | updv::clean<char &>;
    REQUIRE(get<0>(view) == 4);
    REQUIRE(get<1>(view) == 67);
  }

  SECTION("Enumerate a tuple") {
    upd::tuple_view auto view = t | updv::enumerate;
    REQUIRE(get<0>(view) == std::pair{0, 4});
    REQUIRE(&get<0>(view).second == &get<0>(t));
    REQUIRE(get<1>(view) == std::pair{1, 8});
    REQUIRE(&get<1>(view).second == &get<1>(t));
    REQUIRE(get<2>(view) == std::pair{2, 67});
    REQUIRE(&get<2>(view).second == &get<2>(t));
  }

  SECTION("Filter elements of a tuple") {
    upd::tuple_view auto view = t | updv::filter([]<typename T>(upd::typebox<T>) { return !std::same_as<T, char &>; });
    REQUIRE(&get<0>(view) == &get<0>(t));
    REQUIRE(&get<1>(view) == &get<2>(t));
  }

  SECTION("Collect a view into a regular record") {
    upd::regular_tuple auto regt = t | updv::to<std::tuple>;
    REQUIRE(&get<0>(regt) != &get<0>(t));
    REQUIRE(get<0>(regt) == get<0>(t));
    REQUIRE(&get<1>(regt) != &get<1>(t));
    REQUIRE(get<1>(regt) == get<1>(t));
    REQUIRE(&get<2>(regt) != &get<2>(t));
    REQUIRE(get<2>(regt) == get<2>(t));
  }

  SECTION("Zip tuples together") {
    upd::regular_tuple auto t_ = std::tuple{char{78}, long{22}};
    upd::tuple_view auto view = updv::zip(t, t_);
    REQUIRE(&get<0>(view).first == &get<0>(t));
    REQUIRE(&get<0>(view).second == &get<0>(t_));
    REQUIRE(&get<1>(view).first == &get<1>(t));
    REQUIRE(&get<1>(view).second == &get<1>(t_));
    REQUIRE(upd::tuple_size_v<decltype(view)> == 2);
  }
}

TEST_CASE("Algorithms on records", "[tuple_algorithm]") {
  namespace updv = upd::tuple_views;

  upd::regular_tuple auto t = std::tuple{int{4}, char{8}, long{67}};

  SECTION("Find element in a tuple") {
    auto i = updv::find_if(t, []<typename T>(upd::typebox<T>) { return std::same_as<T, long &>; });
    REQUIRE(i == 2);
  }

  SECTION("Left-fold tuple content") {
    auto res = updv::fold_left(t, 0, [i = 0](auto acc, auto v) mutable { return (acc + v) * i++; });

    REQUIRE(res == ((4 * 0 + 8) * 1 + 67) * 2);
  }

  SECTION("Right-fold tuple content") {
    auto res = updv::fold_right(t, 0, [i = 0](auto v, auto acc) mutable { return (acc + i++) * v; });

    REQUIRE(res == ((0 * 67 + 1) * 8 + 2) * 4);
  }
}

TEST_CASE("Tuple view handling references", "[tuple_view]") {
  namespace updv = upd::tuple_views;

  auto lv = 12;
  auto xv = 23;
  auto t = std::tuple<int &, int &&, int>{lv, std::move(xv), 34};

  SECTION("Pass references through transform") {
    upd::tuple_view auto lview = t | updv::transform([](auto &&v) -> auto && { return UPD_FWD(v); });

    REQUIRE_SAME(get<0>(lview), (lv));
    REQUIRE_SAME(get<1>(lview), (xv));
    REQUIRE_SAME(get<2>(lview), get<2>(t));

    upd::tuple_view auto rview = std::move(t) | updv::transform([](auto &&v) -> auto && { return UPD_FWD(v); });

    REQUIRE_SAME(get<0>(rview), (lv));
    REQUIRE_SAME(get<1>(rview), std::move(xv));
    REQUIRE_SAME(get<2>(rview), get<2>(std::move(t)));
  }

  SECTION("Pass references through clean") {
    upd::tuple_view auto lview = t | updv::clean<void>;

    REQUIRE_SAME(get<0>(lview), (lv));
    REQUIRE_SAME(get<1>(lview), (xv));
    REQUIRE_SAME(get<2>(lview), get<2>(t));

    upd::tuple_view auto rview = std::move(t) | updv::clean<void>;

    REQUIRE_SAME(get<0>(rview), (lv));
    REQUIRE_SAME(get<1>(rview), std::move(xv));
    REQUIRE_SAME(get<2>(rview), get<2>(std::move(t)));
  }

  SECTION("Pass references through filter") {
    upd::tuple_view auto lview = t | updv::filter([](auto) { return true; });

    REQUIRE_SAME(get<0>(lview), (lv));
    REQUIRE_SAME(get<1>(lview), (xv));
    REQUIRE_SAME(get<2>(lview), get<2>(t));

    upd::tuple_view auto rview = std::move(t) | updv::filter([](auto) { return true; });

    REQUIRE_SAME(get<0>(rview), (lv));
    REQUIRE_SAME(get<1>(rview), std::move(xv));
    REQUIRE_SAME(get<2>(rview), get<2>(std::move(t)));
  }

  SECTION("Pass references through enumerate") {
    upd::tuple_view auto lview = t | updv::enumerate;

    REQUIRE_SAME(get<0>(lview).second, (lv));
    REQUIRE_SAME(get<1>(lview).second, (xv));
    REQUIRE_SAME(get<2>(lview).second, get<2>(t));

    upd::tuple_view auto rview = std::move(t) | updv::enumerate;

    REQUIRE_SAME(get<0>(rview).second, (lv));
    REQUIRE_SAME(get<1>(rview).second, std::move(xv));
    REQUIRE_SAME(get<2>(rview).second, get<2>(std::move(t)));
  }

  SECTION("Pass references through zip") {
    auto view = updv::zip(t, std::move(t));

    REQUIRE_SAME(get<0>(view).first, (lv));
    REQUIRE_SAME(get<0>(view).second, (lv));
    REQUIRE_SAME(get<1>(view).first, (xv));
    REQUIRE_SAME(get<1>(view).second, std::move(xv));
    REQUIRE_SAME(get<2>(view).first, get<2>(t));
    REQUIRE_SAME(get<2>(view).second, get<2>(std::move(t)));
  }
}
