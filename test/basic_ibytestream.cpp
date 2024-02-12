// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)                                                                                                \
  if (!(__VA_ARGS__)) {                                                                                                \
    throw std::exception{};                                                                                            \
  }

#include <any>
#include <optional>

#include "utility/generators.hpp"
#include "utility/mocking.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <fakeit.hpp>
#include <upd/basic_tuple.hpp>
#include <upd/detail/assertion.hpp>
#include <upd/static_vector.hpp>

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

template<typename T, T... Xs, typename F>
constexpr auto transform_to_tuple(std::integer_sequence<T, Xs...>, F &&f) noexcept {
  return std::make_tuple(std::invoke(f, integral_constant_t<Xs>{})...);
}

template<std::size_t Begin, std::size_t End, typename Tuple>
constexpr auto subtuple(Tuple &tuple) noexcept {
  constexpr auto subtuple_size = End - Begin;
  constexpr auto seq = std::make_index_sequence<subtuple_size>{};

  auto get_element = [&](auto iconst) {
    auto &element = std::get<Begin + iconst.value>(tuple);
    return std::ref(element);
  };

  return transform_to_tuple(seq, get_element);
}

template<typename T, typename Integral_Constant_T>
struct at_most_t {
  using type = T;
  constexpr static auto max = Integral_Constant_T::value;
};

template<typename, template<typename...> typename>
struct is_instance_of : std::false_type {};

template<template<typename...> typename TT, typename... Ts>
struct is_instance_of<TT<Ts...>, TT> : std::true_type {};

template<typename T, template<typename...> typename TT>
constexpr auto is_instance_of_v = is_instance_of<T, TT>::value;

} // namespace detail

template<typename T, std::size_t Max>
using at_most = detail::at_most_t<T, detail::integral_constant_t<Max>>;

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
  } else if constexpr (detail::is_instance_of_v<T, detail::at_most_t>) {
    return max_serialized_size<typename T::type, Serializer_T>() * T::max;
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

template<typename Producer_T, typename Serializer_T, typename Oracle_T>
class basic_ibytestream {
public:
  explicit basic_ibytestream(Producer_T producer, Serializer_T serializer, Oracle_T oracle) noexcept
      : m_producer{UPD_FWD(producer)}, m_serializer{UPD_FWD(serializer)}, m_oracle{UPD_FWD(oracle)} {}

  template<typename... Ts>
  auto decode(description_t<Ts...>) {
    auto descr = description<Ts...>;
    auto seq = std::make_index_sequence<sizeof...(Ts)>{};
    return decode_description(descr, seq);
  }

  template<typename T>
  auto decode() {
    return decode_with_prefix<T>(std::tuple{});
  }

private:
  template<typename T, typename Prefix_T>
  auto decode_with_prefix(const Prefix_T &prefix) {
    if constexpr (std::is_signed_v<T>) {
      return decode_signed<T>(prefix);
    } else if constexpr (std::is_unsigned_v<T>) {
      return decode_unsigned<T>(prefix);
    } else if constexpr (detail::is_bounded_array_v<T>) {
      return decode_bounded_array<T>(prefix);
    } else if constexpr (detail::is_instance_of_v<T, detail::at_most_t>) {
      return decode_at_most<T>(prefix);
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`T` cannot be serialized");
    }
  }

  template<typename... Ts, std::size_t... Is>
  auto decode_description(description_t<Ts...>, std::index_sequence<Is...>) {
    auto retval = std::tuple<std::optional<decltype(decode<Ts>())>...>{};
    auto deref_and_tie_opts = [](const auto &...opts) {
      UPD_ASSERT((opts && ...));
      return std::tie(*opts...);
    };
    auto deref_opts = [](auto &&...opts) {
      UPD_ASSERT((opts && ...));
      return std::tuple{*std::move(opts)...};
    };

    ((std::get<Is>(retval) = decode_with_prefix<Ts>(std::apply(deref_and_tie_opts, detail::subtuple<0, Is>(retval)))),
     ...);
    return std::apply(deref_opts, std::move(retval));
  }

  template<typename T, typename Prefix_T>
  auto decode_signed(const Prefix_T &) -> T {
    auto buffer = std::array<std::byte, sizeof(T)>{};
    auto adv = [this]() { return advance(); };

    std::generate(buffer.begin(), buffer.end(), adv);

    return m_serializer.deserialize_signed(buffer.data(), buffer.size());
  }

  template<typename T, typename Prefix_T>
  auto decode_unsigned(const Prefix_T &) -> T {
    auto buffer = std::array<std::byte, sizeof(T)>{};
    auto adv = [this]() { return advance(); };

    std::generate(buffer.begin(), buffer.end(), adv);

    return m_serializer.deserialize_unsigned(buffer.data(), buffer.size());
  }

  template<typename T, typename Prefix_T>
  auto decode_bounded_array(const Prefix_T &prefix) {
    using element_t = std::remove_pointer_t<std::decay_t<T>>;

    constexpr auto size = sizeof(T) / sizeof(element_t);
    auto retval = std::array<std::optional<element_t>, size>{};
    auto dec = [&]() { return decode_with_prefix<element_t>(prefix); };
    auto is_valid = [](const auto &opt) { return opt.has_value(); };
    auto deref_opts = [](auto &&...opts) { return std::array{*std::move(opts)...}; };

    std::generate(retval.begin(), retval.end(), dec);
    UPD_ASSERT(std::all_of(retval.begin(), retval.end(), is_valid));

    return std::apply(deref_opts, retval);
  }

  template<typename T, typename Prefix_T>
  auto decode_at_most(const Prefix_T &prefix) {
    using element_t = typename T::type;

    auto retval = upd::static_vector<element_t, T::max>{};
    auto inserter = std::back_inserter(retval);
    auto count = std::invoke(m_oracle, T{}, prefix);
    auto dec = [&]() { return decode<element_t>(); };

    std::generate_n(inserter, count, dec);

    return retval;
  }

  auto advance() -> std::byte { return *m_producer++; }

private:
  Producer_T m_producer;
  Serializer_T m_serializer;
  Oracle_T m_oracle;
};
} // namespace upd

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

TEST_CASE("Deserializing a packet...") {
  auto storage = std::vector<std::byte>{};
  auto storage_begin = storage.begin();
  auto mock_serializer = Mock<serializer_interface>{};
  auto callback = +[](const std::any &, const std::any &) -> std::size_t { throw std::exception{}; };

  auto &producer = storage_begin;
  auto &serializer = mock_serializer.get();
  auto &oracle = callback;
  auto bstream =
      upd::basic_ibytestream<decltype(producer), decltype(serializer), decltype(oracle)>{producer, serializer, oracle};

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
    callback = [](const std::any &query, const std::any &prefix) -> std::size_t {
      const auto &[a, b] = std::any_cast<std::tuple<const int &, const unsigned int &>>(prefix);
      REQUIRE(query.type() == typeid(upd::at_most<short, 3>));
      REQUIRE(a == -64);
      REQUIRE(b == 48);
      return 2;
    };

    When(Method(mock_serializer, deserialize_signed).Using(_, _)).AlwaysDo(iterate(signed_value));
    When(Method(mock_serializer, deserialize_unsigned).Using(_, _)).AlwaysDo(iterate(unsigned_value));
    REQUIRE(bstream.decode(descr) ==
            std::tuple{-64, 48, upd::static_vector<short, 3>{7, -8}, std::array<char, 4>{-1, 2, -3, 4}});
  }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
