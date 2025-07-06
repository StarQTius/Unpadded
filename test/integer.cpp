#include <fakeit.hpp>
#include <upd/integer.hpp>

using namespace fakeit;

template<std::size_t Bitsize, typename Underlying>
struct Catch::StringMaker<upd::extended_integer<Bitsize, Underlying>> {
  constexpr static auto convert = [](auto xn) { return std::to_string(xn.value()); };
};

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

TEST_CASE("Decompose an extended integer") {
  using namespace upd::literals;

  using byte8 = upd::xinteger<5, std::uint8_t>;
  using byte16 = upd::xinteger<10, std::uint16_t>;
  using byte32 = upd::xinteger<2, std::uint32_t>;

  auto x = 0b1011'0000'1001'1100'0111_x;

  REQUIRE(upd::decompose_into_xuint<byte8>(x) == std::array<byte8, 4>{0b00111_x, 0b01110_x, 0b00010_x, 0b10110_x});
  REQUIRE(upd::decompose_into_xuint<byte16>(x) == std::array<byte16, 2>{0b0111000111_x, 0b1011000010_x});
  REQUIRE(upd::decompose_into_xuint<byte32>(x) ==
          std::array<byte32, 10>{0b11_x, 0b01_x, 0b00_x, 0b11_x, 0b01_x, 0b10_x, 0b00_x, 0b00_x, 0b11_x, 0b10_x});
}

TEST_CASE("Recompose an extended integer") {
  using namespace upd::literals;
  
  using byte8 = upd::xinteger<5, std::uint8_t>;
  using byte16 = upd::xinteger<10, std::uint16_t>;
  using byte32 = upd::xinteger<2, std::uint32_t>;

  auto byte8seq = std::array<byte8, 2>{0b01010_x, 0b11101_x};
  REQUIRE(upd::recompose_into_xuint(byte8seq) == 0b11101'01010_x);

  auto byte16seq = std::array<byte16, 3>{0b1011101010_x, 0b1110001010_x, 0b1011011100_x};
  REQUIRE(upd::recompose_into_xuint(byte16seq) == 0b1011011100'1110001010'1011101010_x);

  auto byte32seq = std::array<byte32, 3>{0b11_x, 0b01_x, 0b10_x};
  REQUIRE(upd::recompose_into_xuint(byte32seq) == 0b10'01'11_x);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
