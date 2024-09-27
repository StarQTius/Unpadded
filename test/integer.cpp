// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)                                                                                                \
  if (!(__VA_ARGS__)) {                                                                                                \
    throw std::exception{};                                                                                            \
  }

#define UPD_NOEXCEPT

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

  auto x = upd::xinteger_fit<std::uint32_t>{0b1011'0000'1001'1100'0111}.resize(upd::width<20>).value();

  REQUIRE(upd::decompose_into_xuint<byte8>(x) == std::array<byte8, 4>{0b00111, 0b01110, 0b00010, 0b10110});
  REQUIRE(upd::decompose_into_xuint<byte16>(x) == std::array<byte16, 2>{0b0111000111, 0b1011000010});
  REQUIRE(upd::decompose_into_xuint<byte32>(x) ==
          std::array<byte32, 10>{0b11, 0b01, 0b00, 0b11, 0b01, 0b10, 0b00, 0b00, 0b11, 0b10});
}

TEST_CASE("Recompose an extended integer") {
  using byte8 = upd::xinteger<5, std::uint8_t>;
  using byte16 = upd::xinteger<10, std::uint16_t>;
  using byte32 = upd::xinteger<2, std::uint32_t>;

  auto byte8seq = std::array<byte8, 2>{0b01010, 0b11101};
  REQUIRE(upd::recompose_into_xuint(byte8seq) == 0b11101'01010);

  auto byte16seq = std::array<byte16, 3>{0b1011101010, 0b1110001010, 0b1011011100};
  REQUIRE(upd::recompose_into_xuint(byte16seq) == 0b1011011100'1110001010'1011101010);

  auto byte32seq = std::array<byte32, 3>{0b11, 0b01, 0b10};
  REQUIRE(upd::recompose_into_xuint(byte32seq) == 0b10'01'11);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
