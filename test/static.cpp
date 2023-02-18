#include <cstdint>

#include <upd/detail/type_traits/iterator_category.hpp>
#include <upd/detail/type_traits/signature.hpp>
#include <upd/detail/type_traits/smallest.hpp>

int main() {
  using namespace upd::detail;

  auto f = []() {};

  static_assert(is_input_byte_iterator<std::uint8_t *>::value, "");
  static_assert(is_input_byte_iterator<const std::uint8_t *>::value, "");
  static_assert(is_output_byte_iterator<std::uint8_t *>::value, "");
  static_assert(!is_output_byte_iterator<const std::uint8_t *>::value, "");
  static_assert(!std::is_same<signature_t<decltype(f)>, no_signature>::value, "");
  static_assert(std::is_same<smallest_unsigned_t<(1ull << 8) - 1>, std::uint8_t>::value, "");
  static_assert(std::is_same<smallest_unsigned_t<1ull << 8>, std::uint16_t>::value, "");
  static_assert(std::is_same<smallest_unsigned_t<(1ull << 16) - 1>, std::uint16_t>::value, "");
  static_assert(std::is_same<smallest_unsigned_t<1ull << 16>, std::uint32_t>::value, "");
  static_assert(std::is_same<smallest_unsigned_t<(1ull << 32) - 1>, std::uint32_t>::value, "");
  static_assert(std::is_same<smallest_unsigned_t<1ull << 32>, std::uint64_t>::value, "");

  return 0;
}
