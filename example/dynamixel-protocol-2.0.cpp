#include <cinttypes>
#include <print>
#include <iostream>

#include <upd/description.hpp>
#include <upd/integer.hpp>

namespace std {

ostream &operator<<(ostream &os, byte b) { return os << static_cast<int>(b); }

} // namespace std

using crc = upd::xuint<16>;

template<std::size_t Width, typename Underlying>
struct std::formatter<upd::extended_integer<Width, Underlying>> {
  std::formatter<Underlying> underlying_formatter;

  consteval formatter() noexcept = default;

  constexpr auto parse(std::format_parse_context &ctx) {
    return underlying_formatter.parse(ctx);
  }

  auto format(upd::extended_integer<Width, Underlying> xi, std::format_context &ctx) const {
    return underlying_formatter.format(xi.value(), ctx);
  }
};

template<>
struct std::formatter<upd::no_error> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(upd::no_error, std::format_context &ctx) {
    return std::format_to(ctx.out(), "No error");
  }
};

template<>
struct std::formatter<upd::not_matching_deduction> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(const upd::not_matching_deduction &err, std::format_context &ctx) {
    return std::format_to(ctx.out(), "'{}' field actual and deduced value do not match", err.identifier);
  }
};

template<>
struct std::formatter<upd::invalid_code_in_one_of> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(const upd::invalid_code_in_one_of &err, std::format_context &ctx) {
    return std::format_to(ctx.out(), "{} is not a code of any alternative in one-of field '{}'", err.code, err.identifier);
  }
};

template<>
struct std::formatter<upd::negative_repetition_count> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(const upd::negative_repetition_count &err, std::format_context &ctx) {
    return std::format_to(ctx.out(), "Negative repetition count {} found for repetition field '{}'", err.count, err.identifier);
  }
};

template<>
struct std::formatter<upd::repeated_beyond_max> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(const upd::repeated_beyond_max &err, std::format_context &ctx) {
    return std::format_to(ctx.out(), "Field repeated {} time in '{}', beyond the maximum limit ({})", err.count, err.identifier, err.max);
  }
};

template<>
struct std::formatter<upd::error> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(const upd::error &err, std::format_context &ctx) {
    auto format_error_data = [&](const auto &err_data) {
      return std::format_to(ctx.out(), "{}", err_data);
    };

    return err.visit(format_error_data);
  }
};

template<typename... Ts>
struct std::formatter<std::variant<Ts...>> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(const std::variant<Ts...> &one_of_values, std::format_context &ctx) {
    auto format_alt = [&](const auto &alt) { return std::format_to(ctx.out(), "{}", alt); };
    return std::visit(format_alt, one_of_values);
  }
};

template<auto Identifiers, typename... Ts>
struct std::formatter<upd::named_tuple<Identifiers, Ts...>> {
  consteval formatter() noexcept = default;
  
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(const upd::named_tuple<Identifiers, Ts...> &named_elems, std::format_context &ctx) {
    auto it = ctx.out();
    it = std::format_to(it, "(");
    named_elems.for_each([&, first=true] (const auto &named_elem) mutable {
        if (first) {
          it = std::format_to(it, "{}", named_elem);
          first = false;
        } else {
          it = std::format_to(it, ", {}", named_elem);
        }
    });
    it = std::format_to(it, ")");

    ctx.advance_to(it);
    return it;
  }
};

template<upd::name Identifier, typename T>
struct std::formatter<upd::named_value<Identifier, T>> {
  std::formatter<T> value_formatter;

  consteval formatter() noexcept = default;

  constexpr auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(const upd::named_value<Identifier, T> &named_obj, std::format_context &ctx) {
    return std::format_to(ctx.out(), "{} -> {}", named_obj.identifier.string, named_obj.value());
  }
};

template<typename T, std::size_t Max>
struct std::formatter<upd::static_vector<T, Max>> {
  std::formatter<T> element_formatter;

  consteval formatter() noexcept = default;

  constexpr auto parse(std::format_parse_context &ctx) {
    return element_formatter.parse(ctx);
  }

  auto format(const upd::static_vector<T, Max> &statvec, std::format_context &ctx) const {
    auto it = ctx.out();
    it = std::format_to(it, "{{");
    
    for (auto first=true; const auto &elem : statvec) {
      if (first) {
        first = false;
      } else {
        it = std::format_to(it, ", ");
      }

      ctx.advance_to(it);
      it = element_formatter.format(elem, ctx);
    }

    it = std::format_to(it, "}}");
    
    return it;
  }
};

template<>
struct std::formatter<std::monostate> {
  constexpr auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  auto format(std::monostate, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "<monostate>");
  }
};

constexpr auto accumulate_crc(crc acc, upd::xuint<8> byte) noexcept -> crc {
  constexpr auto crc_table = std::array {
      0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
      0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
      0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
      0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
      0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
      0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
      0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
      0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,
      0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
      0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
      0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
      0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
      0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
      0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
      0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
      0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,
      0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
      0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
      0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
      0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
      0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
      0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
      0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
      0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,
      0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
      0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
      0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
      0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
      0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
      0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
      0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
      0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202
  };

  auto i = ((acc >> 8) ^ byte) & 0xff;
  return *((acc << 8) ^ upd::extended_integer{crc_table[i.value()]}).resize(upd::width<16>);
}

enum class instruction_code {
  ping = 0x1,
  read = 0x2,
  write = 0x3,
  reg_write = 0x4,
  action = 0x5,
  factory_reset = 0x6,
  reboot = 0x8,
  status = 0x55,
};

enum class factory_reset_target {
  all = 0xff,
  all_but_id = 0x1,
  all_but_id_and_baudrate = 0x2,
};

template<>
struct std::formatter<instruction_code> {
  using underlying_type = std::underlying_type_t<instruction_code>;

  std::formatter<underlying_type> underlying_formatter;

  consteval formatter() noexcept = default;

  constexpr auto parse(std::format_parse_context &ctx) {
    return underlying_formatter.parse(ctx);
  }

  auto format(instruction_code code, std::format_context &ctx) const {
    auto underlying_value = std::to_underlying(code);
    return underlying_formatter.format(underlying_value, ctx);
  }
};

constexpr auto description = [] {
  using namespace upd;
  using namespace upd::literals;
  using namespace upd::descriptor;

  using enum instruction_code;

  return constant<"header">(0x00fdffff_x, width<32>)
  | field<"id">(unsigned_int, width<8>)
  | bound<"length">(unsigned_int, width<16>, length_of<"parameters"> / 8_x + 3_x)
  | bound<"instruction">(enumeration<instruction_code>, width<8>)
  | one_of<"parameters">(value_of<"instruction">,
    when<ping> = empty_description,
    when<read> = field<"address">(unsigned_int, width<16>)
               | field<"length">(unsigned_int, width<16>),
    when<write> = field<"address">(unsigned_int, width<16>)
                | repeat<"data">(
                    field(unsigned_int, width<8>),
                    value_of<"length"> - 3_x,
                    at_most<1024>
                ),
    when<reg_write> = field<"address">(unsigned_int, width<16>)
                | repeat<"data">(
                    field(unsigned_int, width<8>),
                    value_of<"length"> - 3_x,
                    at_most<1024>
                ),
    when<action> = empty_description,
    when<factory_reset> = field(enumeration<factory_reset_target>, width<8>),
    when<reboot> = empty_description
  )
  | checksum<"crc">(accumulate_crc, *(0_x).resize(width<16>), all_fields);
}();

constexpr auto answer_description = [] {
  using namespace upd;
  using namespace upd::literals;
  using namespace upd::descriptor;

  using enum instruction_code;

  return constant<"header">(0x00fdffff_x, width<32>)
    | field<"id">(unsigned_int, width<8>)
    | bound<"length">(unsigned_int, width<16>, length_of<"parameters"> / 8_x + 4_x)
    | constant<"instruction">(0x55_x, width<8>)
    | field<"error">(unsigned_int, width<8>)
    | one_of<"parameters">(value_of<"status_of">,
      when<ping> = field<"model_number">(unsigned_int, width<16>)
                 | field<"firmware_version">(unsigned_int, width<8>),
      when<read> = repeat<"data">(
        field(unsigned_int, width<8>),
        value_of<"length"> - 4_x,
        at_most<1024>
      ),
      when<write> = empty_description,
      when<reg_write> = empty_description,
      when<action> = empty_description,
      when<factory_reset> = empty_description,
      when<reboot> = empty_description
    )
    | checksum<"crc">(accumulate_crc, *(0_x).resize(width<16>), all_fields);
}();

struct serializer {
  using byte_type = std::byte;

  constexpr static auto bytewidth = std::numeric_limits<unsigned char>::digits;

  template<typename XInteger, typename OutputIt>
  void serialize_unsigned(XInteger value, OutputIt output) {
    namespace stdr = std::ranges;

    static_assert(upd::is_extended_integer_v<XInteger>, "`value` must be an instance of `extended_integer`");
    static_assert(!upd::is_signed_v<XInteger>, "`value` must be unsigned");

    constexpr auto byte_count = value.bitsize / bytewidth;

    auto decomposition = value.decompose(upd::width<byte_count>);
    stdr::transform(decomposition, output, [](auto xint) { return static_cast<std::byte>(xint); });
  }

  template<typename XInteger, typename OutputIt>
  void serialize_signed(XInteger value, OutputIt output) {
    using namespace upd::literals;

    namespace stdr = std::ranges;

    static_assert(upd::is_extended_integer_v<XInteger>, "`value` must be an instance of `extended_integer`");
    static_assert(upd::is_signed_v<XInteger>, "`value` must be signed");

    auto sign = value.signbit();
    auto abs = value.abs().enlarge(upd::width<1>);

    if (sign) {
      abs = ~abs + 1_x;
    }

    constexpr auto byte_count = abs.bitsize / bytewidth;

    auto decomposition = abs.decompose(upd::width<byte_count>);
    stdr::transform(decomposition, output, [](auto xint) { return static_cast<std::byte>(xint); });
  }

  template<typename InputIt, std::size_t Bitsize>
  auto deserialize_unsigned(InputIt input, upd::width_t<Bitsize>) {
    static_assert(Bitsize % bytewidth == 0, "`Bitsize` must be a multiple of `bytewidth`");

    constexpr auto size = Bitsize / bytewidth;

    auto byteseq = std::array<std::byte, size>{};
    auto last_written = std::copy_n(input, size, byteseq.begin());

    UPD_ASSERT(last_written == byteseq.end());

    return upd::recompose_into_xuint(byteseq);
  }

  template<typename InputIt, std::size_t Bitsize>
  auto deserialize_signed(InputIt input, upd::width_t<Bitsize>) {
    using namespace upd::literals;

    static_assert((Bitsize + 1) % bytewidth == 0, "`Bitsize` must be a multiple of `bytewidth`");

    constexpr auto size = (Bitsize + 1) / bytewidth;

    auto byteseq = std::array<std::byte, size>{};
    auto last_written = std::copy_n(input, size, byteseq.begin());

    UPD_ASSERT(last_written == byteseq.end());

    auto raw = upd::recompose_into_xuint(byteseq);
    auto sign = ((raw & upd::nth_bit<Bitsize>) != 0);
    auto abs = (sign) ? ~raw + 1_x : raw;

    return sign ? -abs : abs.as_signed();
  }

  void checkpoint(std::string_view) {}
};

template<std::size_t N>
struct bytearray : std::array<std::byte, N> {
  template<typename... Bytes>
  explicit bytearray(Bytes... bytes) noexcept : std::array<std::byte, N>{static_cast<std::byte>(bytes)...} {}
};

template<typename... Bytes>
explicit bytearray(Bytes...) noexcept->bytearray<sizeof...(Bytes)>;

auto ping_example() -> upd::error;
auto read_example() -> upd::error;
auto write_example() -> upd::error;
auto reg_write_example() -> upd::error;
auto action_example() -> upd::error;
auto factory_reset_example() -> upd::error;
auto reboot_example() -> upd::error;

auto main() -> int {
  auto examples = std::array {
    ping_example,
    read_example,
    write_example,
    reg_write_example,
    action_example,
    factory_reset_example,
    reboot_example
  };

  for (auto ex : examples) {
    auto err = ex();
    if (err) {
      std::println("Example resulted in the following error: {}", err);
    }
  }
}

auto ping_example() -> upd::error {
  using namespace upd::literals;

  auto ser = serializer{};
  auto oit = std::ostream_iterator<std::byte>{std::cout, " "};
  std::cout << std::hex;

  std::println("Ping: example 1");
  description.encode(("id"_kw = 1_x, "parameters"_kw = upd::choice<instruction_code::ping>()), ser, oit);
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x07, 0x00, 0x55, 0x00, 0x06, 0x04, 0x26, 0x65, 0x5d};
  auto answer1 = answer_description.decode(answer1_seq.begin(), upd::named_tuple{"status_of"_kw = instruction_code::ping}, ser);
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer 1:");
  std::println("- id: {:x}", (*answer1)["id"_kw]);
  std::println("- length: {:x}", (*answer1)["length"_kw]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw]);
  std::println("- error: {:x}", (*answer1)["error"_kw]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw]);
  std::println("");

  auto answer2_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x02, 0x07, 0x00, 0x55, 0x00, 0x06, 0x04, 0x26, 0x6f, 0x6d};
  auto answer2 = answer_description.decode(answer2_seq.begin(), upd::named_tuple{"status_of"_kw = instruction_code::ping}, ser);
  if (!answer2) {
    return answer2.error();
  }

  std::println("Answer 2:");
  std::println("- id: {:x}", (*answer2)["id"_kw]);
  std::println("- length: {:x}", (*answer2)["length"_kw]);
  std::println("- instruction: {:x}", (*answer2)["instruction"_kw]);
  std::println("- error: {:x}", (*answer2)["error"_kw]);
  std::println("- parameters: {}", (*answer2)["parameters"_kw]);
  std::println("");

  return upd::no_error{};
}

auto read_example() -> upd::error {
  using namespace upd::literals;

  auto ser = serializer{};
  auto oit = std::ostream_iterator<std::byte>{std::cout, " "};
  std::cout << std::hex;

  std::println("Read: example");
  description.encode(("id"_kw = 1_x, "parameters"_kw = upd::choice<instruction_code::read>("address"_kw = 0x84_x, "length"_kw = 4_x)), ser, oit);
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x08, 0x00, 0x55, 0x00, 0xa6, 0x00, 0x00, 0x00, 0x8c, 0xc0};
  auto answer1 = answer_description.decode(answer1_seq.begin(), upd::named_tuple{"status_of"_kw = instruction_code::read}, ser);
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw]);
  std::println("- length: {:x}", (*answer1)["length"_kw]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw]);
  std::println("- error: {:x}", (*answer1)["error"_kw]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw]);
  std::println("");

  return upd::no_error{};
}

auto write_example() -> upd::error {
  using namespace upd::literals;

  auto ser = serializer{};
  auto oit = std::ostream_iterator<std::byte>{std::cout, " "};
  std::cout << std::hex;

  std::println("Write: example");
  description.encode((
    "id"_kw = 1_x,
    "parameters"_kw = upd::choice<instruction_code::write>(
      "address"_kw = 0x74_x,
      "data"_kw = (0x200_x).resize(upd::width<32>)->decompose(upd::width<4>)
    )
  ), ser, oit);
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(), upd::named_tuple{"status_of"_kw = instruction_code::write}, ser);
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw]);
  std::println("- length: {:x}", (*answer1)["length"_kw]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw]);
  std::println("- error: {:x}", (*answer1)["error"_kw]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw]);
  std::println("");

  return upd::no_error{};
}

auto reg_write_example() -> upd::error {
  using namespace upd::literals;

  auto ser = serializer{};
  auto oit = std::ostream_iterator<std::byte>{std::cout, " "};
  std::cout << std::hex;

  std::println("Reg Write: example");
  description.encode((
    "id"_kw = 1_x,
    "parameters"_kw = upd::choice<instruction_code::reg_write>(
      "address"_kw = 0x68_x,
      "data"_kw = (0xc8_x).resize(upd::width<32>)->decompose(upd::width<4>)
    )
  ), ser, oit);
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(), upd::named_tuple{"status_of"_kw = instruction_code::reg_write}, ser);
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw]);
  std::println("- length: {:x}", (*answer1)["length"_kw]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw]);
  std::println("- error: {:x}", (*answer1)["error"_kw]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw]);
  std::println("");

  return upd::no_error{};
}

auto action_example() -> upd::error {
  using namespace upd::literals;

  auto ser = serializer{};
  auto oit = std::ostream_iterator<std::byte>{std::cout, " "};
  std::cout << std::hex;

  std::println("Action: example");
  description.encode((
    "id"_kw = 1_x,
    "parameters"_kw = upd::choice<instruction_code::action>()
  ), ser, oit);
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(), upd::named_tuple{"status_of"_kw = instruction_code::reg_write}, ser);
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw]);
  std::println("- length: {:x}", (*answer1)["length"_kw]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw]);
  std::println("- error: {:x}", (*answer1)["error"_kw]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw]);
  std::println("");

  return upd::no_error{};
}

auto factory_reset_example() -> upd::error {
  using namespace upd::literals;

  auto ser = serializer{};
  auto oit = std::ostream_iterator<std::byte>{std::cout, " "};
  std::cout << std::hex;

  std::println("Action: example");
  description.encode((
    "id"_kw = 1_x,
    "parameters"_kw = upd::choice<instruction_code::factory_reset>(factory_reset_target::all_but_id)
  ), ser, oit);
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(), upd::named_tuple{"status_of"_kw = instruction_code::reg_write}, ser);
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw]);
  std::println("- length: {:x}", (*answer1)["length"_kw]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw]);
  std::println("- error: {:x}", (*answer1)["error"_kw]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw]);
  std::println("");

  return upd::no_error{};
}

auto reboot_example() -> upd::error {
  using namespace upd::literals;

  auto ser = serializer{};
  auto oit = std::ostream_iterator<std::byte>{std::cout, " "};
  std::cout << std::hex;

  std::println("Reboot: example");
  description.encode((
    "id"_kw = 1_x,
    "parameters"_kw = upd::choice<instruction_code::reboot>()
  ), ser, oit);
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(), upd::named_tuple{"status_of"_kw = instruction_code::reg_write}, ser);
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw]);
  std::println("- length: {:x}", (*answer1)["length"_kw]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw]);
  std::println("- error: {:x}", (*answer1)["error"_kw]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw]);
  std::println("");

  return upd::no_error{};
}
