// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)                                                                                                \
  if (!(__VA_ARGS__)) {                                                                                                \
    throw std::exception{};                                                                                            \
  }

#include <any>
#include <iterator>
#include <optional>
#include <utility>
#include <variant>

#include "mock/serializer_interface.hpp"
#include "utility/generators.hpp"
#include "utility/mocking.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <fakeit.hpp>
#include <upd/basic_ibytestream.hpp>
#include <upd/basic_obytestream.hpp>
#include <upd/basic_tuple.hpp>
#include <upd/detail/assertion.hpp>
#include <upd/detail/variadic/map.hpp>
#include <upd/detail/variadic/max.hpp>
#include <upd/static_vector.hpp>

using namespace fakeit;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

TEST_CASE("Serializing a packet...") {
  auto storage = std::vector<std::byte>{};
  auto mock_serializer = Mock<serializer_interface>{};

  auto consumer = std::back_inserter(storage);
  auto &serializer = mock_serializer.get();
  auto bstream = upd::basic_obytestream<decltype(consumer), decltype(serializer)>{consumer, serializer};

  When(Method(mock_serializer, serialize_signed)).AlwaysReturn();
  When(Method(mock_serializer, serialize_unsigned)).AlwaysReturn();

  SECTION("...containing integers and array of integer") {
    int int_array[] = {-1, 2, -3, 4};
    unsigned long ulong_array[] = {100, 200};
    bstream.encode(int{-64}, (unsigned int)48, int_array, (unsigned short)16, ulong_array, char{1});

    Verify(Method(mock_serializer, serialize_signed).Using(-64, sizeof(int), _),
           Method(mock_serializer, serialize_unsigned).Using(48, sizeof(unsigned int), _),
           Method(mock_serializer, serialize_signed).Using(-1, sizeof(int), _),
           Method(mock_serializer, serialize_signed).Using(2, sizeof(int), _),
           Method(mock_serializer, serialize_signed).Using(-3, sizeof(int), _),
           Method(mock_serializer, serialize_signed).Using(4, sizeof(int), _),
           Method(mock_serializer, serialize_unsigned).Using(16, sizeof(unsigned short), _),
           Method(mock_serializer, serialize_unsigned).Using(100, sizeof(unsigned long), _),
           Method(mock_serializer, serialize_unsigned).Using(200, sizeof(unsigned long), _),
           Method(mock_serializer, serialize_signed).Using(1, sizeof(char), _));
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
