#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "detail/always_false.hpp"
#include "detail/assertion.hpp"
#include "detail/convert_placeholder.hpp"
#include "detail/fail_on_invoke.hpp"
#include "detail/instantiate_from.hpp"
#include "detail/integral_constant.hpp"
#include "detail/is_bounded_array.hpp" // IWYU pragma: keep
#include "detail/is_instance_of.hpp"   // IWYU pragma: keep
#include "detail/tuple_operations.hpp"
#include "detail/variadic/at.hpp"
#include "detail/variadic/map.hpp"
#include "detail/variadic/max.hpp" // IWYU pragma: keep
#include "detail/variadic/sum.hpp" // IWYU pragma: keep
#include "placeholder.hpp"
#include "static_vector.hpp"
#include "upd.hpp"

namespace upd {

template<typename T, typename Serializer_T>
[[nodiscard]] constexpr static auto max_serialized_size() noexcept -> std::size_t {
  if constexpr (std::is_integral_v<T>) {
    return sizeof(T);
  } else if constexpr (detail::is_bounded_array_v<T>) {
    using element_t = std::remove_pointer_t<std::decay_t<T>>;
    constexpr auto count = sizeof(T) / sizeof(element_t);
    constexpr auto size = max_serialized_size<element_t, Serializer_T>();
    return count * size;
  } else if constexpr (detail::is_instance_of_v<T, detail::at_most_t>) {
    return max_serialized_size<typename T::type, Serializer_T>() * T::max;
  } else if constexpr (detail::is_instance_of_v<T, one_of>) {
    auto integral_max_size = [](auto &&x) {
      using type = std::remove_reference_t<std::remove_cv_t<decltype(x)>>;
      constexpr auto retval = max_serialized_size<type, Serializer_T>();
      return detail::integral_constant_t<retval>{};
    };

    using max_sizes = detail::variadic::mapf_t<typename T::alternative_ts, decltype(integral_max_size)>;
    return detail::variadic::max_v<max_sizes>;
  } else if constexpr (detail::is_instance_of_v<T, group>) {
    auto integral_max_size = [](auto &&x) {
      using type = std::remove_reference_t<std::remove_cv_t<decltype(x)>>;
      constexpr auto retval = max_serialized_size<type, Serializer_T>();
      return detail::integral_constant_t<retval>{};
    };

    using max_sizes = detail::variadic::mapf_t<typename T::types, decltype(integral_max_size)>;
    return detail::variadic::sum_v<max_sizes>;
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
  [[nodiscard]] auto decode(description_t<Ts...> descr) {
    return decode(descr, detail::fail_on_invoke);
  }

  template<typename... Ts, typename Oracle_T>
  [[nodiscard]] auto decode(description_t<Ts...>, Oracle_T &oracle) {
    auto descr = description<Ts...>;
    auto seq = std::make_index_sequence<sizeof...(Ts)>{};
    return decode_description(descr, seq, oracle);
  }

  template<typename T>
  [[nodiscard]] auto decode() {
    return decode<T>(detail::fail_on_invoke);
  }

  template<typename T, typename Oracle_T>
  [[nodiscard]] auto decode(Oracle_T &oracle) {
    return decode_with_prefix<T>(oracle, std::tuple{});
  }

private:
  template<typename T, typename Oracle_T, typename... Prefix_Ts>
  [[nodiscard]] auto decode_with_prefix(Oracle_T &oracle, const Prefix_Ts &...prefixes) {
    if constexpr (std::is_signed_v<T>) {
      return decode_signed<T>();
    } else if constexpr (std::is_unsigned_v<T>) {
      return decode_unsigned<T>();
    } else if constexpr (detail::is_bounded_array_v<T>) {
      return decode_bounded_array<T>(oracle, prefixes...);
    } else if constexpr (detail::is_instance_of_v<T, detail::at_most_t>) {
      return decode_at_most<T>(oracle, prefixes...);
    } else if constexpr (detail::is_instance_of_v<T, one_of>) {
      return decode_one_of<T>(oracle, prefixes...);
    } else if constexpr (detail::is_instance_of_v<T, group>) {
      return decode_group<T>(oracle, prefixes...);
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`T` cannot be serialized");
    }
  }

  template<typename... Ts, std::size_t... Is, typename Oracle_T>
  [[nodiscard]] auto decode_description(description_t<Ts...>, std::index_sequence<Is...> seq, Oracle_T &oracle) {
    using converted_ts = std::tuple<detail::convert_placeholder_t<Ts>...>;

    auto dec = [&](auto prefix) {
      auto index_iconst = std::tuple_size<decltype(prefix)>{};
      using type = detail::variadic::at_t<std::tuple<Ts...>, index_iconst>;
      return decode_with_prefix<type>(oracle, prefix);
    };

    return detail::accumulate_tuple<converted_ts>(seq, dec);
  }

  template<typename T>
  [[nodiscard]] auto decode_signed() -> T {
    auto buffer = std::array<std::byte, sizeof(T)>{};
    auto adv = [this]() { return advance(); };

    std::generate(buffer.begin(), buffer.end(), adv);

    return m_serializer.deserialize_signed(buffer.data(), buffer.size());
  }

  template<typename T>
  [[nodiscard]] auto decode_unsigned() -> T {
    auto buffer = std::array<std::byte, sizeof(T)>{};
    auto adv = [this]() { return advance(); };

    std::generate(buffer.begin(), buffer.end(), adv);

    return m_serializer.deserialize_unsigned(buffer.data(), buffer.size());
  }

  template<typename T, typename Oracle_T, typename... Prefix_Ts>
  [[nodiscard]] auto decode_bounded_array(Oracle_T &oracle, const Prefix_Ts &...prefixes) {
    using element_t = std::remove_pointer_t<std::decay_t<T>>;
    using converted_element_t = detail::convert_placeholder_t<element_t>;

    constexpr auto size = sizeof(T) / sizeof(element_t);
    auto retval = std::array<std::optional<converted_element_t>, size>{};
    auto dec = [&, i = std::size_t{0}]() mutable { return decode_with_prefix<element_t>(oracle, prefixes..., i++); };
    auto is_valid = [](const auto &opt) { return opt.has_value(); };
    auto deref_opts = [](auto &&...opts) { return std::array{*std::move(opts)...}; };

    std::generate(retval.begin(), retval.end(), dec);
    UPD_ASSERT(std::all_of(retval.begin(), retval.end(), is_valid));

    return std::apply(deref_opts, retval);
  }

  template<typename T, typename Oracle_T, typename... Prefix_Ts>
  [[nodiscard]] auto decode_at_most(Oracle_T &oracle, const Prefix_Ts &...prefixes) {
    using element_t = typename T::type;
    using converted_element_t = detail::convert_placeholder_t<element_t>;

    auto retval = upd::static_vector<converted_element_t, T::max>{};
    auto inserter = std::back_inserter(retval);
    auto count = std::invoke(oracle, T{}, prefixes...);
    auto dec = [&, i = std::size_t{0}]() mutable { return decode_with_prefix<element_t>(oracle, prefixes..., i++); };

    std::generate_n(inserter, count, dec);

    return retval;
  }

  template<typename T, typename Oracle_T, typename... Prefix_Ts>
  [[nodiscard]] auto decode_one_of(Oracle_T &oracle, const Prefix_Ts &...prefixes) {
    using converted_alternative_ts = detail::variadic::map_t<typename T::alternative_ts, detail::convert_placeholder_t>;

    auto index = std::invoke(oracle, T{}, prefixes...);
    auto seq = std::make_index_sequence<T::alternative_count>{};
    auto generate_variant_maker = [](auto iconst) {
      using alternative_t = std::tuple_element_t<iconst.value, typename T::alternative_ts>;
      using converted_alternative_t = std::tuple_element_t<iconst.value, converted_alternative_ts>;
      using variant_t = detail::instantiate_from_t<std::variant, converted_alternative_ts>;

      return +[](basic_ibytestream &self, Oracle_T &oracle, const Prefix_Ts &...prefixes) {
        return variant_t{std::in_place_type<converted_alternative_t>,
                         self.decode_with_prefix<alternative_t>(oracle, prefixes...)};
      };
    };
    auto variant_makers = detail::transform_to_array(seq, generate_variant_maker);

    return std::invoke(variant_makers.at(index), *this, oracle, prefixes...);
  }

  template<typename T, typename Oracle_T, typename... Prefix_Ts>
  [[nodiscard]] auto decode_group(Oracle_T &oracle, const Prefix_Ts &...prefixes) {
    using converted_ts = detail::variadic::map_t<typename T::types, detail::convert_placeholder_t>;
    auto seq = std::make_index_sequence<std::tuple_size_v<converted_ts>>{};

    auto dec = [&](auto prefix) {
      auto index_iconst = std::tuple_size<decltype(prefix)>{};
      using converted_t = detail::variadic::at_t<converted_ts, index_iconst>;
      return decode_with_prefix<converted_t>(oracle, prefixes..., prefix);
    };

    return detail::accumulate_tuple<converted_ts>(seq, dec);
  }

  [[nodiscard]] auto advance() -> std::byte { return *m_producer++; }

private:
  Producer_T m_producer;
  Serializer_T m_serializer;
};

} // namespace upd
