#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <string_view>
#include <variant>

#include <catch2/catch_test_macros.hpp>
#include <upd/algebra.hpp>
#include <upd/description.hpp>
#include <upd/description_v2.hpp>
#include <upd/error.hpp>
#include <upd/record.hpp>
#include <upd/static_vector.hpp>
#include <upd/stream_interface.hpp>
#include <upd/token.hpp>
#include <upd/tuple_v2.hpp>

#define BITMASK(N) ((1u << N) - 1u)
#define NTH_BIT(N) (1u << N)

struct serializer {
  using byte_type = std::byte;

  constexpr static auto bytewidth = std::numeric_limits<unsigned char>::digits;

  template<std::size_t Bitsize>
  void serialize_unsigned(std::uintmax_t value, upd::width_t<Bitsize>, upd::stream_interface &dest) {
    namespace stdr = std::ranges;

    static_assert(Bitsize % bytewidth == 0);

    auto buf = std::array<upd::word_t, Bitsize / bytewidth>{};
    stdr::generate(buf, [&] {
      auto byte = value & BITMASK(bytewidth);
      value >>= bytewidth;
      return static_cast<upd::word_t>(byte);
    });

    (void)dest.write(buf.data(), buf.size());
  }

  template<std::size_t Bitsize>
  void serialize_signed(std::intmax_t value, upd::width_t<Bitsize>, upd::stream_interface &dest) {
    static_assert((Bitsize + 1) % bytewidth == 0);

    auto signbit = std::signbit(value);
    auto abs = static_cast<std::uintmax_t>(std::abs(value));

    if (signbit) {
      abs = ~abs + 1;
    }

    serialize_unsigned(value, upd::width<Bitsize + 1>, dest);
  }

  template<std::size_t Bitsize>
  auto deserialize_unsigned(upd::stream_interface &src, upd::width_t<Bitsize>) -> std::uintmax_t {
    namespace stdr = std::ranges;
    namespace stdv = std::views;

    static_assert(Bitsize % bytewidth == 0);

    auto retval = std::uintmax_t{0};
    auto buf = std::array<upd::word_t, Bitsize / bytewidth>{};
    (void)src.read(buf.size(), buf.data());
    for (auto w : stdv::reverse(buf)) {
      retval <<= bytewidth;
      retval |= w;
    }

    return retval;
  }

  template<std::size_t Bitsize>
  auto deserialize_signed(upd::stream_interface &src, upd::width_t<Bitsize>) -> std::intmax_t {
    auto raw = deserialize_unsigned(src, upd::width<Bitsize + 1>);
    auto sign = ((raw & NTH_BIT(Bitsize)) != 0);
    auto abs = static_cast<std::intmax_t>(((sign) ? ~raw + 1 : raw) & BITMASK(Bitsize));

    return (sign) ? -abs : abs;
  }

  void checkpoint(std::string_view) {}
};

TEST_CASE("Protocol descriptors", "[descriptor]") {
  using namespace upd::descriptor;
  using namespace upd::record_operators;
  using namespace upd::literals;

  auto buf = std::array<upd::word_t, 64>{};
  auto ser = serializer{};
  auto st = upd::iterator_stream{buf.begin(), buf.begin()};

  SECTION("Encode then decode unsigned field") {
    auto descr = ufield2<"abc", 16>;
    descr.encode(("abc"_kw2 = 42), ser, st);
    REQUIRE((*descr.decode(st, ser))["abc"_kw2] == 42);
  }

  SECTION("Encode then decode signed field") {
    auto descr = field2<"abc", 15>;
    descr.encode(("abc"_kw2 = 42), ser, st);
    REQUIRE((*descr.decode(st, ser))["abc"_kw2] == 42);
    descr.encode(("abc"_kw2 = -8), ser, st);
    REQUIRE((*descr.decode(st, ser))["abc"_kw2] == -8);
  }

  SECTION("Encode then decode enumeration field") {
    enum class abc {
      a = -34,
      b = 5,
      c = 56,
    };

    auto descr = efield2<"abc", abc, 16>;
    descr.encode(("abc"_kw2 = abc::a), ser, st);
    REQUIRE((*descr.decode(st, ser))["abc"_kw2] == abc::a);
    descr.encode(("abc"_kw2 = abc::b), ser, st);
    REQUIRE((*descr.decode(st, ser))["abc"_kw2] == abc::b);
    descr.encode(("abc"_kw2 = abc::c), ser, st);
    REQUIRE((*descr.decode(st, ser))["abc"_kw2] == abc::c);
  }

  SECTION("Encode then decode an anonymous unsigned field") {
    auto descr = ufield2<upd::anon, 16>;
    descr.encode(42, ser, st);
    REQUIRE(*descr.decode(st, ser) == 42);
  }

  SECTION("Encode then decode an anonymous signed field") {
    auto descr = field2<upd::anon, 15>;
    descr.encode(42, ser, st);
    REQUIRE(*descr.decode(st, ser) == 42);
    descr.encode(-8, ser, st);
    REQUIRE(*descr.decode(st, ser) == -8);
  }

  SECTION("Encode then decode unsigned field") {
    auto descr = ufield2<"def", 16> | ubound2<"abc", 16>(upd::value_of<"def"> * 3);
    descr.encode(("def"_kw2 = 10), ser, st);
    REQUIRE((*descr.decode(st, ser))["abc"_kw2] == 30);
  }

  SECTION("Encode then decode signed field") {
    auto descr = field2<"def", 15> | bound2<"abc", 15>(upd::value_of<"def"> - 10);
    descr.encode(("def"_kw2 = 5), ser, st);
    REQUIRE((*descr.decode(st, ser))["abc"_kw2] == -5);
    descr.encode(("def"_kw2 = 18), ser, st);
    REQUIRE((*descr.decode(st, ser))["abc"_kw2] == 8);
  }

  SECTION("Encode and decode a repeated field") {
    auto descr = ufield2<"len", 16> | repeat<"abc">(ufield2<"def", 16>, upd::value_of<"len">);
    descr.encode(
        ("len"_kw2 = 3 * 16,
         "abc"_kw2 = std::array{upd::record{"def"_kw2 = 4}, upd::record{"def"_kw2 = 8}, upd::record{"def"_kw2 = 16}}),
        ser,
        st);

    auto result = *descr.decode(st, ser);
    REQUIRE(result["len"_kw2] == 3 * 16);
    REQUIRE(result["abc"_kw2][0]["def"_kw2] == 4);
    REQUIRE(result["abc"_kw2][1]["def"_kw2] == 8);
    REQUIRE(result["abc"_kw2][2]["def"_kw2] == 16);
  }

  SECTION("Encode and decode a checksum field") {
    auto descr = ufield2<"abc", 16>
                 | ufield2<"def", 16>
                 | checksum2<"ghi", 16>([](auto acc, auto v) { return acc + v; }, all_fields);
    descr.encode(("abc"_kw2 = 54, "def"_kw2 = 46), ser, st);

    auto result = *descr.decode(st, ser);
    REQUIRE(result["abc"_kw2] == 54);
    REQUIRE(result["def"_kw2] == 46);
    REQUIRE(result["ghi"_kw2] == 100);
  }

  SECTION("Encode and decode a constant field") {
    auto descr = constant2<"abc", 32>(0x12345678);
    descr.encode(upd::record{}, ser, st);

    auto result = *descr.decode(st, ser);
    REQUIRE(result["abc"_kw2] == 0x12345678);
  }

  SECTION("Encode and decode a constant enumeration field") {
    enum class abc { a = 67 };

    auto descr = constant2<"abc", 16>(abc::a);
    descr.encode(upd::record{}, ser, st);

    auto result = *descr.decode(st, ser);
    REQUIRE(result["abc"_kw2] == abc::a);
  }

  SECTION("Encode and decode an expanding repeated field") {
    auto descr = ubound2<"len", 16>(upd::length_of<"abc">) | repeat<"abc">(ufield2<upd::anon, 16>);
    descr.encode(("abc"_kw2 = std::array{4, 8, 16}), ser, st);

    auto result = *descr.decode(st, ser);
    REQUIRE(result["len"_kw2] == 3 * 16);
    REQUIRE(result["abc"_kw2].size() == 3);
    REQUIRE(result["abc"_kw2][0] == 4);
    REQUIRE(result["abc"_kw2][1] == 8);
    REQUIRE(result["abc"_kw2][2] == 16);
  }

  SECTION("Encode and decode a one-of field") {
    enum class abc { a, b, c };
    auto descr = efield2<"i", abc, 16>
                 | one_of<"alts">(upd::value_of<"i">,
                                  upd::when<abc::a> = constant2<"k", 8>(34),
                                  upd::when<abc::b> = constant2<"k", 8>(45),
                                  upd::when<abc::c> = constant2<"k", 8>(56));

    descr.encode(("i"_kw2 = abc::a), ser, st);
    auto result = *descr.decode(st, ser);
    REQUIRE(result["i"_kw2] == abc::a);
    REQUIRE(std::get<1>(result["alts"_kw2])["k"_kw2] == 34);

    descr.encode(("i"_kw2 = abc::b), ser, st);
    result = *descr.decode(st, ser);
    REQUIRE(result["i"_kw2] == abc::b);
    REQUIRE(std::get<2>(result["alts"_kw2])["k"_kw2] == 45);

    descr.encode(("i"_kw2 = abc::c), ser, st);
    result = *descr.decode(st, ser);
    REQUIRE(result["i"_kw2] == abc::c);
    REQUIRE(std::get<3>(result["alts"_kw2])["k"_kw2] == 56);
  }

  SECTION("Encode then decode a shadow enumeration field") {
    enum class abc {
      a = -34,
      b = 5,
      c = 56,
    };

    auto descr = shadow_efield2<"abc", abc, 16>
                 | one_of<"alts">(upd::value_of<"abc">,
                                  upd::when<abc::a> = constant2<"k", 8>(34),
                                  upd::when<abc::b> = constant2<"k", 8>(45),
                                  upd::when<abc::c> = constant2<"k", 8>(56));

    descr.encode(("abc"_kw2 = abc::a), ser, st);
    auto res1 = *descr.decode(st, ser, ("abc"_kw2 = abc::a));
    REQUIRE(res1["abc"_kw2] == abc::a);
    REQUIRE(std::get<1>(res1["alts"_kw2])["k"_kw2] == 34);

    descr.encode(("abc"_kw2 = abc::b), ser, st);
    auto res2 = *descr.decode(st, ser, ("abc"_kw2 = abc::b));
    REQUIRE(res2["abc"_kw2] == abc::b);
    REQUIRE(std::get<2>(res2["alts"_kw2])["k"_kw2] == 45);

    descr.encode(("abc"_kw2 = abc::c), ser, st);
    auto res3 = *descr.decode(st, ser, ("abc"_kw2 = abc::c));
    REQUIRE(res3["abc"_kw2] == abc::c);
    REQUIRE(std::get<3>(res3["alts"_kw2])["k"_kw2] == 56);
  }

  SECTION("Encode and decode a constant and a checksum field") {
    auto descr = constant2<"abc", 8>(12)
                 | constant2<"def", 8>(88)
                 | checksum2<"ghi", 16>([](auto acc, auto v) { return acc + v; }, all_fields);
    descr.encode(upd::record{}, ser, st);

    auto result = *descr.decode(st, ser);
    REQUIRE(result["abc"_kw2] == 12);
    REQUIRE(result["def"_kw2] == 88);
    REQUIRE(result["ghi"_kw2] == 100);
  }

  SECTION("Try decode a checksum field when mismatch") {
    auto descr = ufield2<"abc", 16>
                 | ufield2<"def", 16>
                 | checksum2<"ghi", 16>([](auto acc, auto v) { return acc + v; }, all_fields);

    buf[0] = 54;
    buf[1] = 0;
    buf[2] = 46;
    buf[3] = 0;
    buf[4] = 101;
    buf[5] = 0;

    auto result = descr.decode(st, ser);
    REQUIRE_FALSE(result);
    REQUIRE(result.error()
            == upd::checksum_mismatch{
                .actual = 101,
                .expected = 100,
            });
  }
}

TEST_CASE("Nested protocol descriptors", "[descriptor]") {
  using namespace upd::descriptor;
  using namespace upd::record_operators;
  using namespace upd::literals;

  auto buf = std::array<upd::word_t, 64>{};
  auto ser = serializer{};
  auto st = upd::iterator_stream{buf.begin(), buf.begin()};

  SECTION("Expanding repeated field in a one-of field") {
    enum class abc { a };

    auto descr = ubound2<"len", 16>(upd::length_of<"param">)
                 | efield2<"i", abc, 16>
                 | one_of<"param">(upd::value_of<"i">, upd::when<abc::a> = repeat<"abc">(ufield2<upd::anon, 16>));

    descr.encode(("i"_kw2 = abc::a, "param"_kw2 = ("abc"_kw2 = std::array{4, 8, 16})), ser, st);

    auto result = *descr.decode(st, ser);
    REQUIRE(result["len"_kw2] == 3 * 16);
    REQUIRE(result["i"_kw2] == abc::a);
    REQUIRE(std::get<1>(result["param"_kw2])["abc"_kw2].size() == 3);
    REQUIRE(std::get<1>(result["param"_kw2])["abc"_kw2][0] == 4);
    REQUIRE(std::get<1>(result["param"_kw2])["abc"_kw2][1] == 8);
    REQUIRE(std::get<1>(result["param"_kw2])["abc"_kw2][2] == 16);
  }
}
