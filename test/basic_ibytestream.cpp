// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)                                                                                                \
  if (!(__VA_ARGS__)) {                                                                                                \
    throw std::exception{};                                                                                            \
  }

#include <array>
#include <cstddef>
#include <exception>
#include <iterator>
#include <memory>
#include <vector>

#include "mock/serializer_interface.hpp"
#include "utility/mocking.hpp"
#include <catch2/catch_test_macros.hpp>
#include <fakeit.hpp>
#include <upd/basic_ibytestream.hpp>
#include <upd/description.hpp>
#include <upd/detail/assertion.hpp>
#include <upd/static_vector.hpp>

using namespace fakeit;

template<typename Value_T>
struct input_iterator_interface {
  using difference_type = std::size_t;
  using value_type = Value_T;
  using pointer = Value_T *;
  using reference = Value_T &;
  using iterator_category = std::input_iterator_tag;

  [[nodiscard]] virtual auto operator==(const input_iterator_interface &) const -> bool;
  [[nodiscard]] virtual auto operator*() const -> Value_T;
  virtual auto operator++() -> input_iterator_interface &;
  virtual ~input_iterator_interface() = default;

  input_iterator_interface(const input_iterator_interface &) = default;
  input_iterator_interface(input_iterator_interface &&) noexcept = default;

  auto operator=(const input_iterator_interface &) -> input_iterator_interface & = default;
  auto operator=(input_iterator_interface &&) noexcept -> input_iterator_interface & = default;

  auto operator++(int) {
    auto retval = **this;
    ++(*this);
    return std::make_unique<decltype(retval)>(std::move(retval));
  }
};

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

TEST_CASE("Deserializing a packet...") {
  auto mock_producer = Mock<input_iterator_interface<std::byte>>{};
  auto mock_serializer = Mock<serializer_interface>{};

  auto &producer = mock_producer.get();
  auto &serializer = mock_serializer.get();
  auto bstream = upd::basic_ibytestream<decltype(producer), decltype(serializer)>{producer, serializer};

  SECTION("...containing only integers") {
    using namespace upd::literals;

    auto descr =
        upd::field<"a"_h>(upd::signed_int, upd::width<16>) | upd::field<"b"_h>(upd::unsigned_int, upd::width<8>) |
        upd::field<"c"_h>(upd::unsigned_int, upd::width<16>) | upd::field<"d"_h>(upd::signed_int, upd::width<8>);
    auto signed_value = std::array{-64, 1};
    auto unsigned_value = std::array{48, 16};

    When(Method(mock_producer, operator*)).AlwaysReturn(std::byte{0});
    When(OverloadedMethod(mock_producer, operator++, input_iterator_interface<std::byte> & ())).AlwaysReturn(producer);
    When(Method(mock_serializer, deserialize_signed)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned)).AlwaysDo(iterate(unsigned_value));

    auto decoded = bstream.decode(descr);

    REQUIRE(decoded.get<"a"_h>() == -64);
    REQUIRE(decoded.get<"b"_h>() == 48);
    REQUIRE(decoded.get<"c"_h>() == 16);
    REQUIRE(decoded.get<"d"_h>() == 1);
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
