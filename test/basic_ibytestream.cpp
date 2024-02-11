#include <optional>

#include "utility/generators.hpp"
#include "utility/mocking.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <fakeit.hpp>
#include <upd/basic_tuple.hpp>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...) REQUIRE(__VA_ARGS__)

using namespace fakeit;

struct serializer_interface {
  serializer_interface(const serializer_interface &) = delete;
  auto operator=(const serializer_interface &) -> serializer_interface & = delete;

  serializer_interface(serializer_interface &&) = delete;
  auto operator=(serializer_interface &&) -> serializer_interface & = delete;

  virtual void serialize_unsigned(unsigned long long value, std::size_t size, std::byte *output) = 0;
  virtual void serialize_signed(long long value, std::size_t size, std::byte *output) = 0;
  virtual auto deserialize_unsigned(const std::byte *input, std::size_t size) -> unsigned long long = 0;
  virtual auto deserialize_signed(const std::byte *input, std::size_t size) -> long long = 0;
  virtual ~serializer_interface() = default;
};

namespace upd {
namespace detail {
template<typename>
struct is_bounded_array : std::false_type {};

template<typename T, std::size_t N>
struct is_bounded_array<T[N]> : std::true_type {};

template<typename T>
constexpr auto is_bounded_array_v = is_bounded_array<T>::value;
} // namespace detail

template<typename T, typename Serializer_T>
constexpr static auto max_serialized_size() noexcept -> std::size_t {
  if constexpr (std::is_integral_v<T>) {
    return sizeof(T);
  } else if constexpr (detail::is_bounded_array_v<T>) {
    using element_t = std::remove_pointer_t<std::decay_t<T>>;
    constexpr auto count = sizeof(T) / sizeof(element_t);
    constexpr auto size = max_serialized_size<element_t, Serializer_T>();
    return count * size;
  } else {
    static_assert(UPD_ALWAYS_FALSE, "Type is not serializable");
  }
}

template<typename... Ts>
struct description_t {
  template<typename Serializer_T>
  constexpr static auto max_serialized_size() noexcept -> std::size_t {
    return (0 + ... + upd::max_serialized_size<Ts, Serializer_T>());
  }
};

template<typename... Ts>
constexpr auto description = description_t<Ts...>{};

template<typename Producer_T, typename Serializer_T>
class basic_ibytestream {
public:
  explicit basic_ibytestream(Producer_T producer, Serializer_T serializer) noexcept
      : m_producer{UPD_FWD(producer)}, m_serializer{UPD_FWD(serializer)} {}

  template<typename... Ts>
  auto decode(description_t<Ts...>) {
    auto descr = description<Ts...>;
    auto seq = std::make_index_sequence<sizeof...(Ts)>{};
    return decode_description(descr, seq);
  }

  template<typename T>
  auto decode() {
    if constexpr (std::is_signed_v<T>) {
      return decode_signed<T>();
    } else if constexpr (std::is_unsigned_v<T>) {
      return decode_unsigned<T>();
    } else if constexpr (detail::is_bounded_array_v<T>) {
      return decode_bounded_array<T>();
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`T` cannot be serialized");
    }
  }

private:
  template<typename... Ts, std::size_t... Is>
  auto decode_description(description_t<Ts...>, std::index_sequence<Is...>) {
    auto retval = std::tuple<std::optional<decltype(decode<Ts>())>...>{};
    auto deref_opts = [](auto &&...opts) { return std::tuple{*std::move(opts)...}; };
    auto are_valid = [](const auto &...opts) { return (opts && ...); };

    ((std::get<Is>(retval) = decode<Ts>()), ...);
    UPD_ASSERT(std::apply(are_valid, retval));

    return std::apply(deref_opts, std::move(retval));
  }

  template<typename T>
  auto decode_signed() -> T {
    auto buffer = std::array<std::byte, sizeof(T)>{};
    auto adv = [this]() { return advance(); };

    std::generate(buffer.begin(), buffer.end(), adv);

    return m_serializer.deserialize_signed(buffer.data(), buffer.size());
  }

  template<typename T>
  auto decode_unsigned() -> T {
    auto buffer = std::array<std::byte, sizeof(T)>{};
    auto adv = [this]() { return advance(); };

    std::generate(buffer.begin(), buffer.end(), adv);

    return m_serializer.deserialize_unsigned(buffer.data(), buffer.size());
  }

  template<typename T>
  auto decode_bounded_array() {
    using element_t = std::remove_pointer_t<std::decay_t<T>>;

    constexpr auto size = sizeof(T) / sizeof(element_t);
    auto retval = std::array<std::optional<element_t>, size>{};
    auto dec = [this]() { return decode<element_t>(); };
    auto is_valid = [](const auto &opt) { return opt.has_value(); };
    auto deref_opts = [](auto &&...opts) { return std::array{*std::move(opts)...}; };

    std::generate(retval.begin(), retval.end(), dec);
    UPD_ASSERT(std::all_of(retval.begin(), retval.end(), is_valid));

    return std::apply(deref_opts, retval);
  }

  auto advance() -> std::byte { return *m_producer++; }

private:
  Producer_T m_producer;
  Serializer_T m_serializer;
};
} // namespace upd

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
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
