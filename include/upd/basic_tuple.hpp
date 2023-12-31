#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/serialization.hpp"
#include "detail/type_traits/require.hpp"
#include "detail/type_traits/signature.hpp"
#include "detail/variadic/at.hpp"
#include "detail/variadic/clip.hpp"
#include "detail/variadic/sum.hpp" // IWYU pragma: keep
#include "format.hpp"
#include "type.hpp"
#include "upd.hpp"
#include "upd/index_type.hpp"

#define UPD_ALWAYS_FALSE ([] {}, false)

namespace upd {

namespace detail {

template<auto Value>
using integral_constant_t = std::integral_constant<decltype(Value), Value>;

template<typename, typename = void>
struct is_iterable : std::false_type {};

template<typename T>
struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>>
    : std::true_type {};

template<typename T>
constexpr inline bool is_iterable_v = is_iterable<T>::value;

} // namespace detail

template<typename T, detail::require_is_serializable<T> = 0>
constexpr auto serialization_size_impl(...) -> std::size_t {
  return sizeof(T);
}

template<typename T, detail::require_is_user_serializable<T> = 0>
constexpr auto serialization_size_impl(int) -> std::size_t {
  using namespace upd::detail;

  return decltype(make_view_for<endianess::LITTLE, signed_mode::TWOS_COMPLEMENT>(
      (byte_t *)nullptr, examine_invocable<decltype(upd_extension<T>::unserialize)>{}))::size;
}

template<typename T>
using serialization_size_t = detail::integral_constant_t<serialization_size_impl<T>(0)>;

template<typename Storage_T, typename Serializer_T, typename... Ts>
class basic_tuple {
  static_assert((!(std::is_const_v<Ts> || std::is_volatile_v<Ts> || std::is_reference_v<Ts>)&&...),
                "Type parameters cannot be cv-qualified or ref-qualified.");

  static_assert((detail::is_serializable<Ts>::value && ...),
                "Some of the provided types are not serializable (serializable types are integer types, types with "
                "user-defined extension and array types of any of these)");

  using elements_t = std::tuple<Ts...>;
  using sizes_t = std::tuple<detail::integral_constant_t<0u>, serialization_size_t<Ts>...>;

public:
  template<std::size_t I>
  using element_t = detail::variadic::at_t<elements_t, I>;

  explicit basic_tuple(Storage_T storage, Serializer_T serializer, const Ts &...values)
      : m_storage{UPD_FWD(storage)}, m_serializer{UPD_FWD(serializer)} {
    set_all(std::make_index_sequence<sizeof...(Ts)>{}, values...);
  }

  template<std::size_t I>
  [[nodiscard]] auto get(index_type<I>) const noexcept {
    using sizes_up_to_element_t = detail::variadic::clip_t<sizes_t, 0, I + 1>;

    constexpr auto offset = detail::variadic::sum_v<sizes_up_to_element_t>;

    return read_as<detail::variadic::at_t<elements_t, I>>(offset);
  }

  template<typename F>
  auto invoke(F &&f) const -> decltype(auto) {
    return invoke_on_some(UPD_FWD(f), std::make_index_sequence<sizeof...(Ts)>{});
  }

  template<std::size_t I>
  void set(index_type<I>, const element_t<I> &value) noexcept {
    using sizes_up_to_element_t = detail::variadic::clip_t<sizes_t, 0, I + 1>;

    constexpr auto offset = detail::variadic::sum_v<sizes_up_to_element_t>;

    write_as(value, offset);
  }

  template<std::size_t I, typename T, std::size_t N>
  void set(index_type<I>, const std::array<T, N> &value) noexcept {
    static_assert(std::is_same_v<T[N], detail::variadic::at_t<elements_t, I>>, "The `I`th element type is not `T[N]`");

    using sizes_up_to_element_t = detail::variadic::clip_t<sizes_t, 0, I + 1>;

    constexpr auto offset = detail::variadic::sum_v<sizes_up_to_element_t>;

    write_as(value, offset);
  }

private:
  template<typename T>
  [[nodiscard]] auto read_as(std::size_t offset) const noexcept {
    if constexpr (std::is_unsigned_v<T>) {
      return m_serializer.deserialize_unsigned(m_storage.data() + offset, sizeof(T));
    } else if constexpr (std::is_signed_v<T>) {
      return m_serializer.deserialize_signed(m_storage.data() + offset, sizeof(T));
    } else if constexpr (std::is_array_v<T>) {
      using element_t = std::remove_pointer_t<std::decay_t<T>>;
      using retval_t = std::array<element_t, sizeof(T) / sizeof(element_t)>;

      retval_t retval{};
      auto f = [&, i = 0]() mutable {
        auto element = read_as<element_t>(offset + i * sizeof(element_t));
        ++i;

        return element;
      };

      std::generate(retval.begin(), retval.end(), f);

      return retval;
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`T` cannot be serialized");
    }
  }

  template<typename F, std::size_t... Is>
  auto invoke_on_some(F &&f, std::index_sequence<Is...>) const -> decltype(auto) {
    return std::invoke(UPD_FWD(f), get(index_type_v<Is>)...);
  }

  template<std::size_t... Is>
  void set_all(std::index_sequence<Is...>, const Ts &...values) noexcept {
    (set(index_type_v<Is>, values), ...);
  }

  template<typename T>
  void write_as(const T &value, std::size_t offset) noexcept {
    if constexpr (std::is_unsigned_v<T>) {
      return m_serializer.serialize_unsigned(value, sizeof value, m_storage.data() + offset);
    } else if constexpr (std::is_signed_v<T>) {
      return m_serializer.serialize_signed(value, sizeof value, m_storage.data() + offset);
    } else if constexpr (detail::is_iterable_v<T &>) {
      auto begin = std::begin(value);
      auto end = std::end(value);
      auto f = [&, i = 0](const auto &v) mutable {
        write_as(v, offset + i * sizeof v);
        ++i;
      };

      std::for_each(begin, end, f);
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`T` cannot be serialized");
    }
  }

  Storage_T m_storage;
  Serializer_T m_serializer;
};

} // namespace upd

template<typename Storage_T, typename Serializer_T, typename... Ts>
struct std::tuple_size<upd::basic_tuple<Storage_T, Serializer_T, Ts...>> {
  constexpr static std::size_t value = sizeof...(Ts);
};

template<std::size_t I, typename Storage_T, typename Serializer_T, typename... Ts>
struct std::tuple_element<I, upd::basic_tuple<Storage_T, Serializer_T, Ts...>> {
  using tuple_t = upd::basic_tuple<Storage_T, Serializer_T, Ts...>;
  using type = decltype(std::declval<tuple_t>().get(upd::index_type_v<I>));
};
