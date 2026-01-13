#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <limits>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include <upd/algebra.hpp>
#include <upd/description.hpp>
#include <upd/description_v2.hpp>
#include <upd/error.hpp>
#include <upd/record.hpp>
#include <upd/stream_interface.hpp>
#include <upd/token.hpp>

#define BITMASK(N) ((1u << N) - 1u)
#define NTH_BIT(N) (1u << N)

namespace std {

auto operator<<(ostream &os, byte b) -> ostream & { return os << static_cast<int>(b); }

} // namespace std

using crc = std::uint16_t;

template<typename... Ts>
struct std::formatter<std::variant<Ts...>> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const std::variant<Ts...> &one_of_values, std::format_context &ctx) {
    auto format_alt = [&](const auto &alt) { return std::format_to(ctx.out(), "{}", alt); };
    return std::visit(format_alt, one_of_values);
  }
};

template<>
struct std::formatter<std::monostate> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(std::monostate, std::format_context &ctx) const { return std::format_to(ctx.out(), "<monostate>"); }
};

constexpr auto accumulate_crc(crc acc, std::uint8_t byte) noexcept -> crc {
  constexpr auto crc_table = std::array{
      0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011, 0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d,
      0x8027, 0x0022, 0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072, 0x0050, 0x8055, 0x805f, 0x005a,
      0x804b, 0x004e, 0x0044, 0x8041, 0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2, 0x00f0, 0x80f5,
      0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1, 0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
      0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082, 0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d,
      0x8197, 0x0192, 0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1, 0x01e0, 0x81e5, 0x81ef, 0x01ea,
      0x81fb, 0x01fe, 0x01f4, 0x81f1, 0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2, 0x0140, 0x8145,
      0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151, 0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
      0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132, 0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e,
      0x0104, 0x8101, 0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312, 0x0330, 0x8335, 0x833f, 0x033a,
      0x832b, 0x032e, 0x0324, 0x8321, 0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371, 0x8353, 0x0356,
      0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342, 0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
      0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2, 0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd,
      0x83b7, 0x03b2, 0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381, 0x0280, 0x8285, 0x828f, 0x028a,
      0x829b, 0x029e, 0x0294, 0x8291, 0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2, 0x82e3, 0x02e6,
      0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2, 0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
      0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252, 0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e,
      0x0264, 0x8261, 0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231, 0x8213, 0x0216, 0x021c, 0x8219,
      0x0208, 0x820d, 0x8207, 0x0202};

  auto i = ((acc >> 8) ^ byte) & 0xff;
  return (acc << 8) ^ crc_table[i];
}

enum class instruction_code {
  ping = 0x1,
  read = 0x2,
  write = 0x3,
  reg_write = 0x4,
  action = 0x5,
  factory_reset = 0x6,
  reboot = 0x8,
  clear = 0x10,
  control_table_backup = 0x20,
  sync_read = 0x82,
  status = 0x55,
};

enum class factory_reset_target {
  all = 0xff,
  all_but_id = 0x1,
  all_but_id_and_baudrate = 0x2,
};

enum class clear_target : std::uint64_t {
  present_position = 0x224c584401,
  registered_errors = 0x4c43524502,
};

enum class control_table_backup_target : std::uint64_t {
  store_current = 0x4c52544301,
  restore = 0x4c52544302,
};

template<>
struct std::formatter<instruction_code> {
  using underlying_type = std::underlying_type_t<instruction_code>;

  std::formatter<underlying_type> underlying_formatter;

  consteval formatter() noexcept = default;

  constexpr auto parse(std::format_parse_context &ctx) { return underlying_formatter.parse(ctx); }

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

  return constant2<"header", 32>(0x00fdffff) | ufield2<"id", 8> |
         ubound2<"length", 16>(length_of<"parameters"> / 8 + 3) | efield2<"instruction", instruction_code, 8> |
         one_of<"parameters">(value_of<"instruction">,
                              when<ping> = empty_description,
                              when<read> = ufield2<"address", 16> | ufield2<"length_", 16>,
                              when<write> = ufield2<"address", 16> | repeat<"data">(ufield2<anon, 8>),
                              when<reg_write> = ufield2<"address", 16> | repeat<"data">(ufield2<anon, 8>),
                              when<action> = empty_description,
                              when<factory_reset> = efield2<anon, factory_reset_target, 8>,
                              when<reboot> = empty_description,
                              when<clear> = efield2<anon, clear_target, 40>,
                              when<control_table_backup> = efield2<anon, control_table_backup_target, 40>,
                              when<sync_read> =
                                  ufield2<"address", 16> | ufield2<"length_", 16> | repeat<"ids">(ufield2<anon, 8>)) |
         checksum2<"crc", 16>(accumulate_crc, all_fields);
}();

constexpr auto answer_description = [] {
  using namespace upd;
  using namespace upd::literals;
  using namespace upd::descriptor;

  using enum instruction_code;

  return constant2<"header", 32>(0x00fdffff) | ufield2<"id", 8> |
         ubound2<"length", 16>(length_of<"parameters"> / 8 + 4) | constant2<"instruction", 8>(instruction_code::ping) |
         ufield2<"error", 8> |
         one_of<"parameters">(value_of<"status_of">,
                              when<ping> = ufield2<"model_number", 16> | ufield2<"firmware_version", 8>,
                              when<read> = repeat<"data">(ufield2<anon, 8>),
                              when<write> = empty_description,
                              when<reg_write> = empty_description,
                              when<action> = empty_description,
                              when<factory_reset> = empty_description,
                              when<reboot> = empty_description,
                              when<control_table_backup> = empty_description,
                              when<sync_read> = repeat<"data">(ufield2<anon, 8>)) |
         checksum2<"crc", 16>(accumulate_crc, all_fields);
}();

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
    auto abs = static_cast<std::intmax_t>((sign) ? ~raw + 1 : raw);

    return (sign) ? -abs : abs;
  }

  void checkpoint(std::string_view) {}
};

template<std::size_t N>
struct bytearray : std::array<std::byte, N> {
  template<typename... Bytes>
  explicit bytearray(Bytes... bytes) noexcept : std::array<std::byte, N>{static_cast<std::byte>(bytes)...} {}
};

template<typename... Bytes>
explicit bytearray(Bytes...) noexcept -> bytearray<sizeof...(Bytes)>;

auto ping_example() -> upd::error;
auto read_example() -> upd::error;
auto write_example() -> upd::error;
auto reg_write_example() -> upd::error;
auto action_example() -> upd::error;
auto factory_reset_example() -> upd::error;
auto reboot_example() -> upd::error;
auto clear_example() -> upd::error;
auto control_table_backup_example() -> upd::error;
auto sync_read_example() -> upd::error;

auto main() -> int {
  auto examples = std::array{
      ping_example,
      read_example,
      write_example,
      reg_write_example,
      action_example,
      factory_reset_example,
      reboot_example,
      clear_example,
      control_table_backup_example,
      sync_read_example,
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
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Ping: example 1");
  description.encode(("id"_kw2 = 1, "instruction"_kw2 = instruction_code::ping, "parameters"_kw2 = upd::record{}),
                     ser,
                     std::cout,
                     " ");
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x07, 0x00, 0x55, 0x00, 0x06, 0x04, 0x26, 0x65, 0x5d};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::ping},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::ping});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer 1:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  auto answer2_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x02, 0x07, 0x00, 0x55, 0x00, 0x06, 0x04, 0x26, 0x6f, 0x6d};
  auto answer2 = answer_description.decode(answer2_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::ping},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::ping});
  if (!answer2) {
    return answer2.error();
  }

  std::println("Answer 2:");
  std::println("- id: {:x}", (*answer2)["id"_kw2]);
  std::println("- length: {:x}", (*answer2)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer2)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer2)["error"_kw2]);
  std::println("- parameters: {}", (*answer2)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}

auto read_example() -> upd::error {
  using namespace upd::literals;
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Read: example");
  description.encode(("id"_kw2 = 1,
                      "instruction"_kw2 = instruction_code::read,
                      "parameters"_kw2 = ("address"_kw2 = 0x84, "length_"_kw2 = 4)),
                     ser,
                     std::cout,
                     " ");
  std::println("");
  std::println("");

  auto answer1_seq =
      bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x08, 0x00, 0x55, 0x00, 0xa6, 0x00, 0x00, 0x00, 0x8c, 0xc0};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::read},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::read});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}

auto write_example() -> upd::error {
  using namespace upd::literals;
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Write: example");
  description.encode(
      ("id"_kw2 = 1,
       "instruction"_kw2 = instruction_code::write,
       "parameters"_kw2 = ("address"_kw2 = 0x74, "data"_kw2 = std::array<std::uint8_t, 4>{0x0, 0x2, 0x0, 0x0})),
      ser,
      std::cout,
      " ");
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::write},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::write});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}

auto reg_write_example() -> upd::error {
  using namespace upd::literals;
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Reg Write: example");
  description.encode(
      ("id"_kw2 = 1,
       "instruction"_kw2 = instruction_code::reg_write,
       "parameters"_kw2 = ("address"_kw2 = 0x68, "data"_kw2 = std::array<std::uint8_t, 4>{0xc8, 0x0, 0x0, 0x0})),
      ser,
      std::cout,
      " ");
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::reg_write},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::reg_write});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}

auto action_example() -> upd::error {
  using namespace upd::literals;
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Action: example");
  description.encode(("id"_kw2 = 1, "instruction"_kw2 = instruction_code::action, "parameters"_kw2 = upd::record{}),
                     ser,
                     std::cout,
                     " ");
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::reg_write},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::reg_write});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}

auto factory_reset_example() -> upd::error {
  using namespace upd::literals;
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Action: example");
  description.encode(("id"_kw2 = 1,
                      "instruction"_kw2 = instruction_code::factory_reset,
                      "parameters"_kw2 = factory_reset_target::all_but_id),
                     ser,
                     std::cout,
                     " ");
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::reg_write},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::reg_write});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}

auto reboot_example() -> upd::error {
  using namespace upd::literals;
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Reboot: example");
  description.encode(("id"_kw2 = 1, "instruction"_kw2 = instruction_code::reboot, "parameters"_kw2 = upd::record{}),
                     ser,
                     std::cout,
                     " ");
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::reg_write},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::reg_write});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}

auto clear_example() -> upd::error {
  using namespace upd::literals;
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Clear: example");
  description.encode(
      ("id"_kw2 = 1, "instruction"_kw2 = instruction_code::clear, "parameters"_kw2 = clear_target::present_position),
      ser,
      std::cout,
      " ");
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::reg_write},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::reg_write});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}

auto control_table_backup_example() -> upd::error {
  using namespace upd::literals;
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Control Table Backup: example");
  description.encode(("id"_kw2 = 1,
                      "instruction"_kw2 = instruction_code::control_table_backup,
                      "parameters"_kw2 = (control_table_backup_target::store_current)),
                     ser,
                     std::cout,
                     " ");
  std::println("");
  std::println("");

  auto answer1_seq = bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x04, 0x00, 0x55, 0x00, 0xa1, 0x0c};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::reg_write},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::reg_write});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}

auto sync_read_example() -> upd::error {
  using namespace upd::literals;
  using namespace upd::record_operators;

  auto ser = serializer{};
  std::cout << std::hex;

  std::println("Sync Read: example");
  description.encode(
      ("id"_kw2 = 0xfe,
       "instruction"_kw2 = instruction_code::sync_read,
       "parameters"_kw2 = ("address"_kw2 = 0x84, "length_"_kw2 = 0x4, "ids"_kw2 = std::array<std::uint8_t, 2>{1, 2})),
      ser,
      std::cout,
      " ");
  std::println("");
  std::println("");

  auto answer1_seq =
      bytearray{0xff, 0xff, 0xfd, 0x00, 0x01, 0x08, 0x00, 0x55, 0x00, 0xa6, 0x00, 0x00, 0x00, 0x8c, 0xc0};
  auto answer1 = answer_description.decode(answer1_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::sync_read},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::sync_read});
  if (!answer1) {
    return answer1.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer1)["id"_kw2]);
  std::println("- length: {:x}", (*answer1)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer1)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer1)["error"_kw2]);
  std::println("- parameters: {}", (*answer1)["parameters"_kw2]);
  std::println("");

  auto answer2_seq =
      bytearray{0xff, 0xff, 0xfd, 0x00, 0x02, 0x08, 0x00, 0x55, 0x00, 0x1f, 0x08, 0x00, 0x00, 0xba, 0xbe};
  auto answer2 = answer_description.decode(answer2_seq.begin(),
                                           ser,
                                           upd::record{"status_of"_kw2 = instruction_code::sync_read},
                                           std::tuple{upd::value_of<"status_of"> = instruction_code::sync_read});
  if (!answer2) {
    return answer2.error();
  }

  std::println("Answer:");
  std::println("- id: {:x}", (*answer2)["id"_kw2]);
  std::println("- length: {:x}", (*answer2)["length"_kw2]);
  std::println("- instruction: {:x}", (*answer2)["instruction"_kw2]);
  std::println("- error: {:x}", (*answer2)["error"_kw2]);
  std::println("- parameters: {}", (*answer2)["parameters"_kw2]);
  std::println("");

  return upd::no_error{};
}
