#include <upd/integer.hpp>
#include <upd/description.hpp>
#include <iostream>

namespace std {

ostream &operator<<(ostream &os, byte b) {
  return os << static_cast<int>(b);
}

} // namespace std

constexpr auto description = [] {
  using namespace upd;
  using namespace upd::literals;
  using namespace upd::descriptor;

  return 
    field<"header"_h>(unsigned_int, width<32>) |
    field<"id"_h > (unsigned_int, width<8>) |
    field<"length"_h>(unsigned_int, width<16>) |
    field<"instruction"_h>(unsigned_int, width<8>) |
    field<"checksum"_h>(unsigned_int, width<16>);
}();

struct serializer {
  template<typename XInteger, typename OutputIt>
  void serialize_unsigned(XInteger value, OutputIt output) {
    static_assert(upd::is_extended_integer_v<XInteger>, "`value` must be an instance of `extended_integer`");
    static_assert(!upd::is_signed_v<XInteger>, "`value` must be unsigned");

    constexpr auto byte_count = value.bitsize / CHAR_BIT;

    auto decomposition = value.decompose(upd::width<byte_count>);
    std::copy(decomposition.begin(), decomposition.end(), output);
  }

  template<typename XInteger, typename OutputIt>
  void serialize_signed(XInteger value, OutputIt output) {
    static_assert(upd::is_extended_integer_v<XInteger>, "`value` must be an instance of `extended_integer`");
    static_assert(upd::is_signed_v<XInteger>, "`value` must be signed");
 
    auto sign = value.signbit();
    auto abs = value.abs().enlarge(upd::width<1>);

    if (sign) {
      abs = ~abs + 1;
    }

    constexpr auto byte_count = abs.bitsize / CHAR_BIT;

    auto decomposition = abs.decompose(upd::width<byte_count>);
    std::copy(decomposition.begin(), decomposition.end(), output);
  }

  template<typename InputIt, std::size_t Bitsize>
  auto deserialize_unsigned(InputIt input, upd::width_t<Bitsize>) {
    static_assert(Bitsize % CHAR_BIT == 0, "`Bitsize` must be a multiple of `CHAR_BIT`");

    constexpr auto size = Bitsize / CHAR_BIT;

    auto byteseq = std::array<std::byte, size>{};
    auto last_written = std::copy_n(input, size, byteseq.begin());

    UPD_ASSERT(last_written == byteseq.end());

    return upd::recompose_into_xuint(byteseq.begin(), byteseq.end());
  }

  template<typename InputIt, std::size_t Bitsize>
  auto deserialize_signed(InputIt input, upd::width_t<Bitsize>) {
    static_assert((Bitsize + 1) % CHAR_BIT == 0, "`Bitsize` must be a multiple of `CHAR_BIT`");

    constexpr auto size = (Bitsize + 1) / CHAR_BIT;

    auto byteseq = std::array<std::byte, size>{};
    auto last_written = std::copy_n(input, size, byteseq.begin());

    UPD_ASSERT(last_written == byteseq.end());
    
    auto raw = upd::recompose_into_xuint(byteseq);
    auto sign = (raw & upd::nth_bit<Bitsize> != 0);
    auto abs = (raw & upd::nth_bit<Bitsize>) ? ~raw + 1: raw;

    return sign ? -abs : abs;
  }
};

int main() {
  using namespace upd::literals;

  auto ser = serializer{};
  auto oit = std::ostream_iterator<std::byte>{std::cout, " "};
  std::cout << std::hex;

  auto ping_ex1 = description.instantiate(
    upd::kw<"header"_h> = 0xfdffff,
    upd::kw<"id"_h> = 1,
    upd::kw<"length"_h> = 3,
    upd::kw<"instruction"_h> = 1,
    upd::kw<"checksum"_h> = 0x4e19);
  if (!ping_ex1) {
    return (int) ping_ex1.error();
  }

  std::printf("Ping: example 1\n");
  ping_ex1->serialize(ser, oit);
  std::printf("\n");
}