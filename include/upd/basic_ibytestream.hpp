#pragma once

#include <array>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "description.hpp"
#include "detail/always_false.hpp"
#include "detail/integral_constant.hpp"
#include "detail/is_bounded_array.hpp" // IWYU pragma: keep
#include "detail/is_instance_of.hpp"   // IWYU pragma: keep
#include "detail/tuple_operations.hpp"
#include "detail/variadic/concat.hpp"
#include "detail/variadic/count.hpp" // IWYU pragma: keep
#include "detail/variadic/find.hpp"  // IWYU pragma: keep
#include "detail/variadic/max.hpp"   // IWYU pragma: keep
#include "detail/variadic/sum.hpp"   // IWYU pragma: keep
#include "integer.hpp"
#include "upd.hpp"

namespace upd {

template<typename Tuple_T, auto... Ids>
class named_tuple {
  using id_ts = detail::integral_constant_tuple_t<Ids...>;

  template<auto..., typename... Ts>
  friend constexpr auto name_tuple(std::tuple<Ts...>) noexcept;

  template<typename... Named_Tuple_Ts>
  friend constexpr auto concat_named_tuple(Named_Tuple_Ts &&...);

public:
  template<auto Id>
  [[nodiscard]] constexpr auto get() noexcept -> auto & {
    using id_t = detail::integral_constant_t<Id>;
    constexpr auto id_pos = detail::variadic::find_v<id_ts, id_t>;

    return std::get<id_pos>(m_content);
  }

  template<auto Id>
  [[nodiscard]] constexpr auto get() const noexcept -> const auto & {
    using id_t = detail::integral_constant_t<Id>;
    constexpr auto id_pos = detail::variadic::find_v<id_ts, id_t>;

    return std::get<id_pos>(m_content);
  }

private:
  constexpr explicit named_tuple(Tuple_T content, detail::integral_constant_tuple_t<Ids...>)
      : m_content{UPD_FWD(content)} {}

  Tuple_T m_content;
};

template<auto... Ids, typename... Ts>
[[nodiscard]] constexpr auto name_tuple(std::tuple<Ts...> tuple) noexcept {
  using id_ts = detail::integral_constant_tuple_t<Ids...>;

  if constexpr (sizeof...(Ids) != sizeof...(Ts)) {
    static_assert(UPD_ALWAYS_FALSE, "`Ids` and `Ts` must contain the same number of elements");
  } else if constexpr (((detail::variadic::count_v<id_ts, detail::integral_constant_t<Ids>> != 1) || ...)) {
    static_assert(UPD_ALWAYS_FALSE, "Each element in `Ids` must be unique");
  } else {
    return named_tuple{std::move(tuple), id_ts{}};
  }
}

template<typename... Named_Tuple_Ts>
[[nodiscard]] constexpr auto concat_named_tuple(Named_Tuple_Ts &&...named_tuples) {
  using id_ts = detail::variadic::concat_t<typename Named_Tuple_Ts::id_ts...>;

  auto content = std::tuple_cat(UPD_FWD(named_tuples).m_content...);
  auto name_content = [&](auto... id_iconsts) { return name_tuple<id_iconsts.value...>(std::move(content)); };

  return std::apply(name_content, id_ts{});
}

template<typename Producer_T, typename Serializer_T>
class basic_ibytestream {
  using byte_t = std::remove_reference_t<decltype(*std::declval<Producer_T>())>;

  constexpr static auto byte_width = [] {
    if constexpr (detail::is_instance_of_v<byte_t, extended_integer>) {
      return byte_t::width;
    } else if constexpr (std::is_integral_v<byte_t>) {
      return std::numeric_limits<byte_t>::digits;
    } else if constexpr (std::is_enum_v<byte_t>) {
      return std::numeric_limits<std::underlying_type_t<byte_t>>::digits;
    } else {
      static_assert(UPD_ALWAYS_FALSE, "Producers can only produce `extended_integer` values, integral values or enumerators");
    }
  }();

public:
  explicit basic_ibytestream(Producer_T producer, Serializer_T serializer) noexcept
      : m_producer{UPD_FWD(producer)}, m_serializer{UPD_FWD(serializer)} {}

  template<typename... Field_Ts>
  [[nodiscard]] auto decode(const description<Field_Ts...> &descr) {
    return decode_description(descr);
  }

private:
  template<typename Element_T>
  [[nodiscard]] auto decode_element(const Element_T &element) {
    if constexpr (detail::is_instance_of_v<Element_T, description>) {
      return decode_description(element);
    } else if constexpr (detail::is_instance_of_v<Element_T, field_t>) {
      return decode_field(element);
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`T` cannot be serialized");
    }
  }

  template<typename... Field_Ts>
  [[nodiscard]] auto decode_description(const description<Field_Ts...> &descr) {
    auto seq = std::index_sequence_for<Field_Ts...>{};
    auto fields = descr.fields();
    auto dec = [&](auto iconst) {
      auto &field = std::get<iconst>(fields);
      return decode_element(field);
    };

    auto named_tuples = detail::transform_orderly_to_tuple(seq, dec);
    auto concat = [](auto &&...named_tuples) { return concat_named_tuple(UPD_FWD(named_tuples)...); };

    return std::apply(concat, std::move(named_tuples));
  }

  template<typename Field_T>
  [[nodiscard]] auto decode_field(Field_T field) {
    static_assert(field.width % byte_width == 0, "A field width must be a multiple of a byte width");

    constexpr auto field_size = field.width / byte_width;
    constexpr auto id = Field_T::id;

    auto buffer = std::array<byte_t, field_size>{};
    auto adv = [this]() { return advance(); };

    std::generate(buffer.begin(), buffer.end(), adv);

    if constexpr (field.is_signed) {
      auto value = m_serializer.deserialize_signed(buffer.data(), buffer.size());
      auto value_tuple = std::tuple{value};
      return name_tuple<id>(value_tuple);
    } else {
      auto value = m_serializer.deserialize_unsigned(buffer.data(), buffer.size());
      auto value_tuple = std::tuple{value};
      return name_tuple<id>(value_tuple);
    }
  }

  [[nodiscard]] auto advance() {
    auto retval = *m_producer;
    ++m_producer;
    return retval;
  }

  Producer_T m_producer;
  Serializer_T m_serializer;
};

} // namespace upd
