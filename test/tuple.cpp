#include <concepts>
#include <tuple>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <upd/constexpr.hpp>
#include <upd/record.hpp>
#include <upd/tuple_v2.hpp>
#include <upd/type_traits.hpp>

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
    upd::tuple_view auto view = t | updv::clean<char>;
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

  SECTION("Join together elements of a tuple") {
    upd::nested_tuple auto nt = std::tuple{t, t};
    upd::tuple_view auto view = nt | updv::join;
    REQUIRE(&get<0>(view) == &get<0>(get<0>(nt)));
    REQUIRE(&get<1>(view) == &get<1>(get<0>(nt)));
    REQUIRE(&get<2>(view) == &get<2>(get<0>(nt)));
    REQUIRE(&get<3>(view) == &get<0>(get<1>(nt)));
    REQUIRE(&get<4>(view) == &get<1>(get<1>(nt)));
    REQUIRE(&get<5>(view) == &get<2>(get<1>(nt)));
  }
}

TEST_CASE("Tuples compatibility with record facilities", "[tuple][record_view]") {
  namespace updv = upd::record_views;

  upd::regular_record auto t = std::tuple{int{4}, char{8}, long{67}};

  SECTION("Transform element of a tuple") {
    upd::record_view auto view = t | updv::transform([](auto i, auto x) { return x + i; });
    REQUIRE(get<0uz>(view) == 4);
    REQUIRE(get<1uz>(view) == 9);
    REQUIRE(get<2uz>(view) == 69);
  }

  SECTION("Clean elements of a tuple") {
    upd::record_view auto view = t | updv::clean<char>;
    REQUIRE(get<0uz>(view) == 4);
    REQUIRE(!upd::has_tag<1uz>(view));
    REQUIRE(get<2uz>(view) == 67);
  }

  SECTION("Enumerate a tuple") {
    upd::record_view auto view = t | updv::enumerate;
    REQUIRE(get<0uz>(view) == std::pair{0, 4});
    REQUIRE(&get<0uz>(view).second == &get<0>(t));
    REQUIRE(get<1uz>(view) == std::pair{1, 8});
    REQUIRE(&get<1uz>(view).second == &get<1>(t));
    REQUIRE(get<2uz>(view) == std::pair{2, 67});
    REQUIRE(&get<2uz>(view).second == &get<2>(t));
  }

  SECTION("Filter elements of a tuple") {
    upd::record_view auto view =
        t | updv::filter([]<typename T>(auto, upd::typebox<T>) { return !std::same_as<T, char &>; });
    REQUIRE(&get<0uz>(view) == &get<0>(t));
    REQUIRE(!upd::has_tag<1uz>(view));
    REQUIRE(&get<2uz>(view) == &get<2>(t));
  }

  SECTION("Find element in a tuple") {
    auto i = updv::find_if(t, []<typename T>(auto, upd::typebox<T>) { return std::same_as<T, long &>; });
    REQUIRE(i == 2);
  }

  SECTION("Left-fold tuple content") {
    auto res = updv::fold_left(t, 0, [](auto acc, auto i, auto v) { return (acc + v) * i; });

    REQUIRE(res == ((4 * 0 + 8) * 1 + 67) * 2);
  }

  SECTION("Right-fold tuple content") {
    auto res = updv::fold_right(t, 0, [](auto i, auto v, auto acc) { return (acc + i) * v; });

    REQUIRE(res == ((2 * 67 + 1) * 8 + 0) * 4);
  }

  SECTION("Join together elements of a tuple") {
    upd::nested_record auto nt = std::tuple{t, t};
    upd::record_view auto view = nt | updv::join([](auto pi, auto i) { return pi * 10 + i; });
    REQUIRE(&get<0uz>(view) == &get<0>(get<0>(nt)));
    REQUIRE(&get<1uz>(view) == &get<1>(get<0>(nt)));
    REQUIRE(&get<2uz>(view) == &get<2>(get<0>(nt)));
    REQUIRE(&get<10uz>(view) == &get<0>(get<1>(nt)));
    REQUIRE(&get<11uz>(view) == &get<1>(get<1>(nt)));
    REQUIRE(&get<12uz>(view) == &get<2>(get<1>(nt)));
  }
}
