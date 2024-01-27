#include "utility/generators.hpp"
#include "utility/mocking.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <fakeit.hpp>
#include <upd/basic_tuple.hpp>

// NOLINTBEGIN

using namespace fakeit;

struct serializer_interface {
  virtual void serialize_unsigned(unsigned long long value, std::size_t size, std::byte *output) = 0;
  virtual void serialize_signed(long long value, std::size_t size, std::byte *output) = 0;
  virtual unsigned long long deserialize_unsigned(const std::byte *input, std::size_t size) = 0;
  virtual long long deserialize_signed(const std::byte *input, std::size_t size) = 0;
};

TEST_CASE("Testing a basic_tuple that contains a few integers", "[basic_tuple]") {
  using storage_t = std::array<std::byte, sizeof(int) + sizeof(unsigned int) + sizeof(int[4])>;

  Mock<serializer_interface> mock_serializer;
  When(Method(mock_serializer, serialize_signed).Using(0, _, _)).AlwaysReturn();
  When(Method(mock_serializer, serialize_unsigned).Using(0, _, _)).AlwaysReturn();
  storage_t storage{};
  upd::basic_tuple<storage_t &, serializer_interface &, int, unsigned int, int[4]> tuple{
      storage, mock_serializer.get(), {}, {}, {}};
  mock_serializer.Reset();

  SECTION("Get a signed integer") {
    int value = GENERATE(0, 0xabc, -0xabc);

    When(Method(mock_serializer, deserialize_signed).Using(_, sizeof value)).Return(value);
    REQUIRE(tuple.get(upd::index_type_v<0>) == value);
  }

  SECTION("Get an unsigned integer") {
    unsigned int value = GENERATE(0, 0xabc);

    When(Method(mock_serializer, deserialize_unsigned).Using(_, sizeof value)).Return(value);
    REQUIRE(tuple.get(upd::index_type_v<1>) == value);
  }

  SECTION("Get an array") {
    using element_t = std::array<int, 4>;

    auto value = GENERATE_VALUES(element_t, {0, 0, 0, 0}, {10, -20, -30, 40});

    When(Method(mock_serializer, deserialize_signed).Using(_, sizeof(int))).AlwaysDo(iterate(value));

    REQUIRE(tuple.get(upd::index_type_v<2>) == value);
  }

  SECTION("Apply a function") {
    int signed_integer = GENERATE(0, 0xabc, -0xabc);
    unsigned int unsigned_integer = GENERATE(0, 0xabc);
    auto f = [&](const auto &x, const auto &y, const auto &z) {
      std::array<int, 4> array{};
      array.fill(signed_integer);

      REQUIRE(x == signed_integer);
      REQUIRE(y == unsigned_integer);
      REQUIRE(z == array);
    };

    When(Method(mock_serializer, deserialize_signed).Using(_, sizeof(int)))
        .Return(5_Times((long long){signed_integer}));

    When(Method(mock_serializer, deserialize_unsigned).Using(_, sizeof(unsigned int))).Return(unsigned_integer);

    tuple.invoke(f);
  }

  SECTION("Set a signed integer") {
    int value = GENERATE(0, 0xabc, -0xabc);

    When(Method(mock_serializer, serialize_signed).Using(value, sizeof value, _)).AlwaysReturn();
    tuple.set(upd::index_type_v<0>, value);
    Verify(Method(mock_serializer, serialize_signed).Using(value, sizeof value, _));
  }

  SECTION("Set an unsigned integer") {
    unsigned int value = GENERATE(0, 0xabc);

    When(Method(mock_serializer, serialize_unsigned).Using(value, sizeof value, _)).AlwaysReturn();
    tuple.set(upd::index_type_v<1>, value);
    Verify(Method(mock_serializer, serialize_unsigned).Using(value, sizeof value, _));
  }

  SECTION("Set an array") {
    using element_t = std::array<int, 4>;

    auto value = GENERATE_VALUES(element_t, {0, 0, 0, 0}, {10, -20, -30, 40});

    When(Method(mock_serializer, serialize_signed).Using(_, sizeof(int), _)).AlwaysReturn();
    tuple.set(upd::index_type_v<2>, value);

    for (auto v : value) {
      Verify(Method(mock_serializer, serialize_signed).Using(v, sizeof v, _));
    }
  }

  SECTION("Set a value at the right location") {
    auto *writing_location = storage.data() + sizeof(int);
    When(Method(mock_serializer, serialize_unsigned).Using(_, _, writing_location)).Return();
    tuple.set(upd::index_type_v<1>, 0);
    Verify(Method(mock_serializer, serialize_unsigned).Using(_, _, writing_location)).Once();
  }
}

// NOLINTEND
