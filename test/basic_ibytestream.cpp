// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)                                                                                                \
  if (!(__VA_ARGS__)) {                                                                                                \
    throw std::exception{};                                                                                            \
  }

#include <any>
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
#include <upd/basic_tuple.hpp>
#include <upd/detail/assertion.hpp>
#include <upd/detail/variadic/map.hpp>
#include <upd/detail/variadic/max.hpp>
#include <upd/static_vector.hpp>

using namespace fakeit;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

TEST_CASE("Deserializing a packet...") {
  auto storage = std::vector<std::byte>{};
  auto storage_begin = storage.begin();
  auto mock_serializer = Mock<serializer_interface>{};

  auto &producer = storage_begin;
  auto &serializer = mock_serializer.get();
  auto bstream = upd::basic_ibytestream<decltype(producer), decltype(serializer)>{producer, serializer};

  SECTION("...containing only integers") {
    auto descr = upd::description<int, unsigned int, unsigned short, char>;
    auto size = descr.max_serialized_size<serializer_interface>();
    auto signed_value = std::array{-64, 1};
    auto unsigned_value = std::array{48, 16};

    storage.resize(size);
    storage_begin = storage.begin();

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned).Using(_, _)).AlwaysDo(iterate(unsigned_value));
    REQUIRE(bstream.decode(descr) == std::tuple{-64, 48, 16, 1});
  }

  SECTION("...containing integers and array of integer") {
    auto descr = upd::description<int, unsigned int, int[4], unsigned short, unsigned long[2], char>;
    auto size = descr.max_serialized_size<serializer_interface>();
    auto signed_value = std::array{-64, -1, 2, -3, 4, 1};
    auto unsigned_value = std::array{48, 16, 100, 200};

    storage.resize(size);
    storage_begin = storage.begin();

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned).Using(_, _)).AlwaysDo(iterate(unsigned_value));
    REQUIRE(bstream.decode(descr) ==
            std::tuple{-64, 48, std::array<int, 4>{-1, 2, -3, 4}, 16, std::array<unsigned long, 2>{100, 200}, 1});
  }

  SECTION("...containing one variable-sized element") {
    auto descr = upd::description<int, unsigned int, upd::at_most<short, 3>, char[4]>;
    auto size = descr.max_serialized_size<serializer_interface>();
    auto signed_value = std::array{-64, 7, -8, -1, 2, -3, 4};
    auto unsigned_value = std::array{48};

    storage.resize(size);
    storage_begin = storage.begin();

    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores,bugprone-easily-swappable-parameters)
    auto oracle = [](const std::any &query, const std::any &prefix) -> std::size_t {
      const auto &[a, b] = std::any_cast<std::tuple<const int &, const unsigned int &>>(prefix);
      REQUIRE(query.type() == typeid(upd::at_most<short, 3>));
      REQUIRE(a == -64);
      REQUIRE(b == 48);
      return 2;
    };

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned).Using(_, _)).AlwaysDo(iterate(unsigned_value));
    REQUIRE(bstream.decode(descr, oracle) ==
            std::tuple{-64, 48, upd::static_vector<short, 3>{7, -8}, std::array<char, 4>{-1, 2, -3, 4}});
  }

  SECTION("...containing one `one_of` element") {
    auto descr = upd::description<int, unsigned int, upd::one_of<char, unsigned int, short[2]>, char[4]>;
    auto size = descr.max_serialized_size<serializer_interface>();
    auto signed_value = std::array{-64, -1, 2, -3, 4};
    auto unsigned_value = std::array{48, 78};

    storage.resize(size);
    storage_begin = storage.begin();

    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores,bugprone-easily-swappable-parameters)
    auto oracle = [](const std::any &query, const std::any &prefix) -> std::size_t {
      const auto &[a, b] = std::any_cast<std::tuple<const int &, const unsigned int &>>(prefix);
      REQUIRE(query.type() == typeid(upd::one_of<char, unsigned int, short[2]>));
      REQUIRE(a == -64);
      REQUIRE(b == 48);
      return 1;
    };

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned).Using(_, _)).AlwaysDo(iterate(unsigned_value));
    REQUIRE(bstream.decode(descr, oracle) ==
            std::tuple{-64,
                       48,
                       std::variant<char, unsigned int, std::array<short, 2>>{std::in_place_type<unsigned int>, 78},
                       std::array<char, 4>{-1, 2, -3, 4}});
  }

  SECTION("...containing a nested `at_most` in a `one_of`") {
    auto descr = upd::description<int, upd::one_of<upd::at_most<unsigned long, 1>, short[2]>, char[4]>;
    auto size = descr.max_serialized_size<serializer_interface>();
    auto signed_value = std::array{-64, -1, 2, -3, 4};
    auto unsigned_value = std::array{78};

    storage.resize(size);
    storage_begin = storage.begin();

    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores,bugprone-easily-swappable-parameters)
    auto oracle = [](auto query, const auto &...prefixes) -> std::size_t {
      using query_t = decltype(query);

      static_assert(sizeof...(prefixes) == 1);

      if constexpr (upd::detail::is_instance_of_v<query_t, upd::one_of>) {
        return 0;
      } else if constexpr (upd::detail::is_instance_of_v<query_t, upd::detail::at_most_t>) {
        return 1;
      } else {
        static_assert(UPD_ALWAYS_FALSE);
      }
    };

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned).Using(_, _)).AlwaysDo(iterate(unsigned_value));
    REQUIRE(bstream.decode(descr, oracle) ==
            std::tuple{-64,
                       std::variant<upd::static_vector<unsigned long, 1>, std::array<short, 2>>{
                           upd::static_vector<unsigned long, 1>{78}},
                       std::array<char, 4>{-1, 2, -3, 4}});
  }

  SECTION("...containing a nested `one_of` in a `at_most`") {
    auto descr = upd::description<short, upd::at_most<upd::one_of<int, unsigned int>, 3>, char[4]>;
    auto size = descr.max_serialized_size<serializer_interface>();
    auto signed_value = std::array{32, -78, 1, 2, -3, -4};
    auto unsigned_value = std::array{89};

    storage.resize(size);
    storage_begin = storage.begin();

    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores,bugprone-easily-swappable-parameters)
    auto oracle = [](auto query, const auto &...prefixes) -> std::size_t {
      using query_t = decltype(query);

      auto prefix_tuple = std::make_tuple(std::cref(prefixes)...);

      if constexpr (upd::detail::is_instance_of_v<query_t, upd::detail::at_most_t>) {
        static_assert(sizeof...(prefixes) == 1);
        return 2;
      } else if constexpr (upd::detail::is_instance_of_v<query_t, upd::one_of>) {
        const auto &[prefix, index] = prefix_tuple;
        switch (index) {
        case 0:
          return 1;
        case 1:
          return 0;
        default:
          throw std::exception{};
        };
      } else {
        static_assert(UPD_ALWAYS_FALSE);
      }
    };

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned).Using(_, _)).AlwaysDo(iterate(unsigned_value));
    REQUIRE(bstream.decode(descr, oracle) ==
            std::tuple{32,
                       upd::static_vector<std::variant<int, unsigned int>, 3>{
                           std::variant<int, unsigned int>{std::in_place_type<unsigned int>, 89},
                           std::variant<int, unsigned int>{std::in_place_type<int>, -78}},
                       std::array<char, 4>{1, 2, -3, -4}});
  }

  SECTION("...containing a nested `at_most` in a `at_most`") {
    auto descr = upd::description<upd::at_most<upd::at_most<int, 4>, 3>>;
    auto size = descr.max_serialized_size<serializer_interface>();
    auto signed_value = std::array{1, -2, -3, 4, 5, -6};

    storage.resize(size);
    storage_begin = storage.begin();

    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores,bugprone-easily-swappable-parameters)
    auto oracle = [](auto, const auto &...prefixes) -> std::size_t {
      if constexpr (sizeof...(prefixes) == 1) {
        return 3;
      } else if constexpr (sizeof...(prefixes) == 2) {
        const auto &[prefix, index] = std::tuple{prefixes...};
        return index + 1;
      } else {
        static_assert(UPD_ALWAYS_FALSE);
      }
    };

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    REQUIRE(bstream.decode(descr, oracle) ==
            std::tuple{upd::static_vector<upd::static_vector<int, 4>, 3>{upd::static_vector<int, 4>{1},
                                                                         upd::static_vector<int, 4>{-2, -3},
                                                                         upd::static_vector<int, 4>{4, 5, -6}}});
  }

  SECTION("...containing a nested `one_of` in an array") {
    auto descr = upd::description<upd::one_of<int, unsigned int>[4]>;
    auto size = descr.max_serialized_size<serializer_interface>();
    auto signed_value = std::array{64, -64};
    auto unsigned_value = std::array{16, 32};

    storage.resize(size);
    storage_begin = storage.begin();

    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores,bugprone-easily-swappable-parameters)
    auto oracle = [](auto, std::tuple<>, std::size_t index) -> std::size_t { return index % 2; };

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned).Using(_, _)).AlwaysDo(iterate(unsigned_value));
    REQUIRE(bstream.decode(descr, oracle) ==
            std::tuple{std::array{std::variant<int, unsigned int>{std::in_place_type<int>, 64},
                                  std::variant<int, unsigned int>{std::in_place_type<unsigned int>, 16},
                                  std::variant<int, unsigned int>{std::in_place_type<int>, -64},
                                  std::variant<int, unsigned int>{std::in_place_type<unsigned int>, 32}}});
  }

  SECTION("...containing a nested group in a `one_of`") {
    auto descr = upd::description<upd::one_of<upd::group<int, unsigned short>, unsigned int>>;
    auto size = descr.max_serialized_size<serializer_interface>();
    auto signed_value = std::array{-64};
    auto unsigned_value = std::array{16};

    storage.resize(size);
    storage_begin = storage.begin();

    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores,bugprone-easily-swappable-parameters)
    auto oracle = [](auto, std::tuple<>) -> std::size_t { return 0; };

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned).Using(_, _)).AlwaysDo(iterate(unsigned_value));
    REQUIRE(bstream.decode(descr, oracle) == std::tuple{std::variant<std::tuple<int, unsigned short>, unsigned int>{
                                                 std::tuple{int{-64}, (unsigned short)16}}});
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
