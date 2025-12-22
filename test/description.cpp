#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <string_view>

#include <catch2/catch_test_macros.hpp>
#include <upd/description.hpp>
#include <upd/description_v2.hpp>
#include <upd/record.hpp>
#include <upd/stream_interface.hpp>
#include <upd/token.hpp>

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
    REQUIRE(*descr.decode(st, ser, upd::record{}) == 42);
  }

  SECTION("Encode then decode an anonymous signed field") {
    auto descr = field2<upd::anon, 15>;
    descr.encode(42, ser, st);
    REQUIRE(*descr.decode(st, ser, upd::record{}) == 42);
    descr.encode(-8, ser, st);
    REQUIRE(*descr.decode(st, ser, upd::record{}) == -8);
  }
}
