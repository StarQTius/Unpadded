#include <array>
#include <tuple>
#include <utility>
#include <variant>

#include "../include/upd/description/frame_segment.hpp"
#include <catch2/catch_test_macros.hpp>
#include <upd/algebra.hpp>
#include <upd/description.hpp>
#include <upd/error.hpp>
#include <upd/record.hpp>
#include <upd/stream.hpp>
#include <upd/tuple.hpp>
#include <upd/utility/static_vector.hpp>
#include <upd/utility/when_then.hpp>

TEST_CASE("Protocol descriptors", "[descriptor]") {
  using namespace upd::descriptor;
  using namespace upd::record_operators;
  using namespace upd::literals;

  auto buf = std::array<char, 100>{};
  auto st = upd::iterator_stream{buf.begin(), buf.begin()};

  SECTION("Encode then decode unsigned field") {
    auto descr = upd::description{"abc"_kw2 = ufield2<16>};
    REQUIRE(descr.encode(("abc"_kw2 = 42), st));
    REQUIRE(descr.decode(st).value()["abc"_kw2] == 42);
  }

  SECTION("Encode then decode signed field") {
    auto descr = upd::description{"abc"_kw2 = field2<16>};
    REQUIRE(descr.encode(("abc"_kw2 = 42), st));
    REQUIRE(descr.decode(st).value()["abc"_kw2] == 42);
    REQUIRE(descr.encode(("abc"_kw2 = -8), st));
    REQUIRE(descr.decode(st).value()["abc"_kw2] == -8);
  }

  SECTION("Encode then decode enumeration field") {
    enum class abc {
      a = -34,
      b = 5,
      c = 56,
    };

    auto descr = upd::description{"abc"_kw2 = efield2<abc, 16>};
    REQUIRE(descr.encode(("abc"_kw2 = abc::a), st));
    REQUIRE(descr.decode(st).value()["abc"_kw2] == abc::a);
    REQUIRE(descr.encode(("abc"_kw2 = abc::b), st));
    REQUIRE(descr.decode(st).value()["abc"_kw2] == abc::b);
    REQUIRE(descr.encode(("abc"_kw2 = abc::c), st));
    REQUIRE(descr.decode(st).value()["abc"_kw2] == abc::c);
  }

  SECTION("Encode then decode an anonymous unsigned field") {
    auto descr = ufield2<16>;
    REQUIRE(descr.encode<upd::anon>(42, st, std::tuple{}));
    REQUIRE(descr.decode<upd::anon>(st, std::tuple{}).value() == 42);
  }

  SECTION("Encode then decode an anonymous signed field") {
    auto descr = field2<16>;
    REQUIRE(descr.encode<upd::anon>(42, st, std::tuple{}));
    REQUIRE(descr.decode<upd::anon>(st, std::tuple{}).value() == 42);
    REQUIRE(descr.encode<upd::anon>(-8, st, std::tuple{}));
    REQUIRE(descr.decode<upd::anon>(st, std::tuple{}).value() == -8);
  }

  SECTION("Encode then decode unsigned field") {
    auto descr =
        upd::description{"def"_kw2 = ufield2<16>,
                         "abc"_kw2 = ubound2<16>(upd::value_of<"def"> * 3)};
    REQUIRE(descr.encode(("def"_kw2 = 10), st));
    REQUIRE(descr.decode(st).value()["abc"_kw2] == 30);
  }

  SECTION("Encode then decode signed field") {
    auto descr =
        upd::description{"def"_kw2 = field2<16>,
                         "abc"_kw2 = bound2<16>(upd::value_of<"def"> - 10)};
    REQUIRE(descr.encode(("def"_kw2 = 5), st));
    REQUIRE(descr.decode(st).value()["abc"_kw2] == -5);
    REQUIRE(descr.encode(("def"_kw2 = 18), st));
    REQUIRE(descr.decode(st).value()["abc"_kw2] == 8);
  }

  SECTION("Encode and decode a repeated field") {
    auto descr = upd::description{
        "len"_kw2 = ufield2<16>,
        "abc"_kw2 = repeat(("def"_kw2 = ufield2<16>), upd::value_of<"len">)};
    REQUIRE(descr.encode(("len"_kw2 = 3 * 16,
                          "abc"_kw2 = std::array{upd::record{"def"_kw2 = 4},
                                                 upd::record{"def"_kw2 = 8},
                                                 upd::record{"def"_kw2 = 16}}),
                         st));

    auto result = descr.decode(st).value();
    REQUIRE(result["len"_kw2] == 3 * 16);
    REQUIRE(result["abc"_kw2][0]["def"_kw2] == 4);
    REQUIRE(result["abc"_kw2][1]["def"_kw2] == 8);
    REQUIRE(result["abc"_kw2][2]["def"_kw2] == 16);
  }

  SECTION("Encode and decode a checksum field") {
    auto descr = upd::description{
        "abc"_kw2 = ufield2<16>, "def"_kw2 = ufield2<16>,
        "ghi"_kw2 = checksum2<16>([](auto acc, auto v) { return acc + v; },
                                  all_previous_fields)};
    REQUIRE(descr.encode(("abc"_kw2 = 54, "def"_kw2 = 46), st));

    auto result = descr.decode(st).value();
    REQUIRE(result["abc"_kw2] == 54);
    REQUIRE(result["def"_kw2] == 46);
  }

  SECTION("Encode and decode a constant field") {
    auto descr = upd::description{"abc"_kw2 = constant2<32>(0x12345678)};
    REQUIRE(descr.encode(upd::record{}, st));
    REQUIRE_NOTHROW(descr.decode(st).value());
  }

  SECTION("Encode and decode a constant enumeration field") {
    enum class abc { a = 67 };

    auto descr = upd::description{"abc"_kw2 = constant2<16>(abc::a)};
    REQUIRE(descr.encode(upd::record{}, st));

    REQUIRE_NOTHROW(descr.decode(st).value());
  }

  SECTION("Encode and decode an expanding repeated field") {
    auto descr =
        upd::description{"len"_kw2 = ubound2<16>(upd::length_of<"abc">),
                         "abc"_kw2 = repeat(ufield2<16>)};
    REQUIRE(descr.encode(("abc"_kw2 = std::array{4, 8, 16}), st));

    auto result = descr.decode(st).value();
    REQUIRE(result["len"_kw2] == 3 * 16);
    REQUIRE(result["abc"_kw2].size() == 3);
    REQUIRE(result["abc"_kw2][0] == 4);
    REQUIRE(result["abc"_kw2][1] == 8);
    REQUIRE(result["abc"_kw2][2] == 16);
  }

  SECTION("Encode and decode a one-of field") {
    enum class abc { a, b, c };
    auto descr = upd::description{
        "i"_kw2 = efield2<abc, 16>,
        "alts"_kw2 = one_of(
            upd::value_of<"i">,
            upd::when<abc::a> = upd::description{"k"_kw2 = constant2<8>(34)},
            upd::when<abc::b> = upd::description{"k"_kw2 = constant2<8>(45)},
            upd::when<abc::c> = upd::description{"k"_kw2 = constant2<8>(56)})};

    REQUIRE(descr.encode(("i"_kw2 = abc::a), st));
    auto result = descr.decode(st).value();
    REQUIRE(result["i"_kw2] == abc::a);
    REQUIRE(result["alts"_kw2].index() == 0);

    REQUIRE(descr.encode(("i"_kw2 = abc::b), st));
    result = descr.decode(st).value();
    REQUIRE(result["i"_kw2] == abc::b);
    REQUIRE(result["alts"_kw2].index() == 1);

    REQUIRE(descr.encode(("i"_kw2 = abc::c), st));
    result = descr.decode(st).value();
    REQUIRE(result["i"_kw2] == abc::c);
    REQUIRE(result["alts"_kw2].index() == 2);
  }

  SECTION("Encode then decode a shadow enumeration field") {
    enum class abc {
      a = -34,
      b = 5,
      c = 56,
    };

    auto descr = upd::description{
        "abc"_kw2 = shadow_efield2<abc, 16>,
        "alts"_kw2 = one_of(
            upd::value_of<"abc">,
            upd::when<abc::a> = upd::description{"k"_kw2 = constant2<8>(34)},
            upd::when<abc::b> = upd::description{"k"_kw2 = constant2<8>(45)},
            upd::when<abc::c> = upd::description{"k"_kw2 = constant2<8>(56)})};

    REQUIRE(descr.encode(("abc"_kw2 = abc::a), st));
    auto res1 = descr.decode(st, ("abc"_kw2 = abc::a)).value();
    REQUIRE(res1["abc"_kw2] == abc::a);
    REQUIRE(res1["alts"_kw2].index() == 0);

    REQUIRE(descr.encode(("abc"_kw2 = abc::b), st));
    auto res2 = descr.decode(st, ("abc"_kw2 = abc::b)).value();
    REQUIRE(res2["abc"_kw2] == abc::b);
    REQUIRE(res2["alts"_kw2].index() == 1);

    REQUIRE(descr.encode(("abc"_kw2 = abc::c), st));
    auto res3 = descr.decode(st, ("abc"_kw2 = abc::c)).value();
    REQUIRE(res3["abc"_kw2] == abc::c);
    REQUIRE(res3["alts"_kw2].index() == 2);
  }

  SECTION("Encode and decode a constant and a checksum field") {
    auto descr = upd::description{
        "abc"_kw2 = constant2<8>(12), "def"_kw2 = constant2<8>(88),
        "ghi"_kw2 = checksum2<16>([](auto acc, auto v) { return acc + v; },
                                  all_previous_fields)};
    REQUIRE(descr.encode(upd::record{}, st));
    REQUIRE_NOTHROW(descr.decode(st).value());
    REQUIRE(buf[2] == 0x64);
    REQUIRE(buf[3] == 0);
  }

  SECTION("Try decode a checksum field when mismatch") {
    auto descr = upd::description{
        "abc"_kw2 = ufield2<16>, "def"_kw2 = ufield2<16>,
        "ghi"_kw2 = checksum2<16>([](auto acc, auto v) { return acc + v; },
                                  all_previous_fields)};

    buf[0] = 54;
    buf[1] = 0;
    buf[2] = 46;
    buf[3] = 0;
    buf[4] = 101;
    buf[5] = 0;

    auto result = descr.decode(st);
    REQUIRE_FALSE(result);
    REQUIRE(result.error()
            == upd::checksum_mismatch{
                .actual = 101,
                .expected = 100,
            });
  }

  SECTION("Encode and decode a partial checksum field") {
    auto descr = upd::description{
        "abc"_kw2 = constant2<8>(12), "def"_kw2 = constant2<8>(37),
        "ghi"_kw2 = constant2<8>(88),
        "jkl"_kw2 = checksum2<16>([](auto acc, auto v) { return acc + v; },
                                  only_fields<"abc", "ghi">)};
    REQUIRE(descr.encode(upd::record{}, st));
    REQUIRE_NOTHROW(descr.decode(st).value());
    REQUIRE(buf[3] == 0x64);
    REQUIRE(buf[4] == 0);
  }

  SECTION("Encode and decode a checksum surrounded by constants") {
    auto descr = upd::description{
        "abc"_kw2 = constant2<8>(12), "def"_kw2 = constant2<8>(88),
        "ghi"_kw2 = checksum2<16>([](auto acc, auto v) { return acc + v; },
                                  all_previous_fields),
        "jkl"_kw2 = constant2<8>(37)};
    REQUIRE(descr.encode(upd::record{}, st));
    REQUIRE_NOTHROW(descr.decode(st).value());
    REQUIRE(buf[2] == 0x64);
    REQUIRE(buf[3] == 0);
  }
}

TEST_CASE("Nested protocol descriptors", "[descriptor]") {
  using namespace upd::descriptor;
  using namespace upd::record_operators;
  using namespace upd::literals;

  auto buf = std::array<char, 100>{};
  auto st = upd::iterator_stream{buf.begin(), buf.begin()};

  SECTION("Expanding repeated field in a one-of field") {
    enum class abc { a };

    auto descr = upd::description{
        "len"_kw2 = ubound2<16>(upd::length_of<"param">),
        "i"_kw2 = efield2<abc, 16>,
        "param"_kw2 =
            one_of(upd::value_of<"i">,
                   upd::when<abc::a> = ("abc"_kw2 = repeat(ufield2<16>)))};

    REQUIRE(descr.encode(
        ("i"_kw2 = abc::a, "param"_kw2 = ("abc"_kw2 = std::array{4, 8, 16})),
        st));

    auto result = descr.decode(st).value();
    REQUIRE(result["len"_kw2] == 3 * 16);
    REQUIRE(result["i"_kw2] == abc::a);
    REQUIRE(std::get<0>(result["param"_kw2])["abc"_kw2].size() == 3);
    REQUIRE(std::get<0>(result["param"_kw2])["abc"_kw2][0] == 4);
    REQUIRE(std::get<0>(result["param"_kw2])["abc"_kw2][1] == 8);
    REQUIRE(std::get<0>(result["param"_kw2])["abc"_kw2][2] == 16);
  }
}

TEST_CASE("Frame object", "[descriptor][record]") {
  using namespace upd::literals;

  using frame_type =
      upd::frame<upd::entry<upd::name{"abc"}, int>,
                 upd::entry<upd::name{"def"}, upd::frame_segment<int>>>;

  upd::regular_record auto frm =
      frame_type{"abc"_kw2 = 42, "def"_kw2 = {13, 17, 19}};

  SECTION("Get elements from a frame") {
    REQUIRE(frm["abc"_kw2] == 42);
    REQUIRE(frm["def"_kw2].size() == 3);
    REQUIRE(frm["def"_kw2][0] == 13);
    REQUIRE(frm["def"_kw2][1] == 17);
    REQUIRE(frm["def"_kw2][2] == 19);
  }

  SECTION("Copy a frame") {
    auto frm_ = frm;

    REQUIRE(frm_["def"_kw2].data() != frm["def"_kw2].data());
    REQUIRE(frm_["abc"_kw2] == 42);
    REQUIRE(frm_["def"_kw2].size() == 3);
    REQUIRE(frm_["def"_kw2][0] == 13);
    REQUIRE(frm_["def"_kw2][1] == 17);
    REQUIRE(frm_["def"_kw2][2] == 19);
  }

  SECTION("Move a frame") {
    auto frm_ = std::move(frm);

    REQUIRE(frm_["def"_kw2].data() != frm["def"_kw2].data());
    REQUIRE(frm_["abc"_kw2] == 42);
    REQUIRE(frm_["def"_kw2].size() == 3);
    REQUIRE(frm_["def"_kw2][0] == 13);
    REQUIRE(frm_["def"_kw2][1] == 17);
    REQUIRE(frm_["def"_kw2][2] == 19);
  }
}
