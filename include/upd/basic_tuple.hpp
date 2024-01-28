#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/always_false.hpp"
#include "detail/integral_constant.hpp"
#include "detail/is_iterable.hpp" // IWYU pragma: keep
#include "detail/serialized_size.hpp"
#include "detail/type_traits/remove_cv_ref.hpp"
#include "detail/type_traits/signature.hpp"
#include "detail/variadic/at.hpp"
#include "detail/variadic/clip.hpp"
#include "detail/variadic/sum.hpp" // IWYU pragma: keep
#include "index_type.hpp"
#include "span.hpp"
#include "upd.hpp"

namespace upd {

template<typename Storage_T, typename Serializer_T, typename... Ts>
class basic_tuple {
  static_assert((!(std::is_const_v<Ts> || std::is_volatile_v<Ts> || std::is_reference_v<Ts>)&&...),
                "Type parameters cannot be cv-qualified or ref-qualified.");

  static_assert(((detail::is_serializable_object_v<Ts, Serializer_T> || std::is_integral_v<Ts> ||
                  std::is_array_v<Ts>)&&...),
                "Some of the provided types are not serializable (serializable types are integer types, types with "
                "user-defined extension and array types of any of these)");

  using elements_t = std::tuple<Ts...>;

public:
  template<std::size_t I>
  using element_t = detail::variadic::at_t<elements_t, I>;

  explicit basic_tuple(Storage_T storage, Serializer_T serializer)
      : m_storage{UPD_FWD(storage)}, m_serializer{UPD_FWD(serializer)} {}

  explicit basic_tuple(Storage_T storage, Serializer_T serializer, const Ts &...values)
      : basic_tuple{UPD_FWD(storage), UPD_FWD(serializer)} {
    set_all(std::make_index_sequence<sizeof...(Ts)>{}, values...);
  }

  template<std::size_t I>
  [[nodiscard]] auto get(index_type<I>) const noexcept {
    return read_as<detail::variadic::at_t<elements_t, I>>(offset_for<I>());
  }

  template<typename F>
  auto invoke(F &&f) const -> decltype(auto) {
    return invoke_on_some(UPD_FWD(f), std::make_index_sequence<sizeof...(Ts)>{});
  }

  template<std::size_t I>
  void set(index_type<I>, const element_t<I> &value) noexcept {
    write_as(value, offset_for<I>());
  }

  template<std::size_t I, typename T, std::size_t N>
  void set(index_type<I>, const std::array<T, N> &value) noexcept {
    static_assert(std::is_same_v<T[N], detail::variadic::at_t<elements_t, I>>, "The `I`th element type is not `T[N]`");

    write_as(value, offset_for<I>());
  }

private:
  template<typename T>
  [[nodiscard]] auto read_as(std::size_t offset) const noexcept {
    if constexpr (std::is_unsigned_v<T>) {
      constexpr auto size = detail::serialized_size<T, Serializer_T>();
      const auto *src = m_storage.data() + offset;
      return m_serializer.deserialize_unsigned(src, size);
    } else if constexpr (std::is_signed_v<T>) {
      constexpr auto size = detail::serialized_size<T, Serializer_T>();
      const auto *src = m_storage.data() + offset;
      return m_serializer.deserialize_signed(src, size);
    } else if constexpr (std::is_array_v<T>) {
      using element_t = std::remove_pointer_t<std::decay_t<T>>;
      using retval_t = std::array<element_t, sizeof(T) / sizeof(element_t)>;

      retval_t retval{};
      auto f = [&, reading_offset = offset]() mutable {
        auto element = read_as<element_t>(reading_offset);
        reading_offset += detail::serialized_size<element_t, Serializer_T>();

        return element;
      };

      std::generate(retval.begin(), retval.end(), f);

      return retval;
    } else if constexpr (detail::is_serializable_object_v<T, Serializer_T>) {
      using deserialize_t = decltype(&detail::remove_cv_ref_t<Serializer_T>::template deserialize_object<T>);
      using args_t = typename detail::examine_invocable<deserialize_t>::args;

      auto view = view_from_variadic(offset, (args_t *)nullptr);
      auto deserialize = [&](auto... args) { return m_serializer.template deserialize_object<T>(std::move(args)...); };

      return view.invoke(deserialize);
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`T` cannot be serialized");
    }
  }

  template<template<typename...> typename TT, typename... Us>
  [[nodiscard]] auto view_from_variadic(std::size_t offset, TT<Us...> *) noexcept {
    using byte_span_t = span<std::byte>;
    using retval_t = basic_tuple<byte_span_t, Serializer_T, Us...>;

    constexpr auto size = (detail::serialized_size<Us, Serializer_T>() + ...);

    auto begin = m_storage.data() + offset;
    byte_span_t span{begin, size};

    return retval_t{span, m_serializer};
  }

  template<template<typename...> typename TT, typename... Us>
  [[nodiscard]] auto view_from_variadic(std::size_t offset, TT<Us...> *) const noexcept {
    using byte_span_t = span<const std::byte>;
    using retval_t = basic_tuple<byte_span_t, Serializer_T, Us...>;

    constexpr auto size = (detail::serialized_size<Us, Serializer_T>() + ...);

    auto begin = m_storage.data() + offset;
    byte_span_t span{begin, size};

    return retval_t{span, m_serializer};
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
      constexpr auto size = detail::serialized_size<T, Serializer_T>();
      auto *dest = m_storage.data() + offset;
      return m_serializer.serialize_unsigned(value, size, dest);
    } else if constexpr (std::is_signed_v<T>) {
      constexpr auto size = detail::serialized_size<T, Serializer_T>();
      auto *dest = m_storage.data() + offset;
      return m_serializer.serialize_signed(value, size, dest);
    } else if constexpr (detail::is_iterable_v<T &>) {
      auto begin = std::begin(value);
      auto end = std::end(value);
      auto f = [&, writing_offset = offset](const auto &v) mutable {
        using arg_t = detail::remove_cv_ref_t<decltype(v)>;
        write_as(v, writing_offset);
        writing_offset += detail::serialized_size<arg_t, Serializer_T>();
      };

      std::for_each(begin, end, f);
    } else if constexpr (detail::is_serializable_object_v<T, Serializer_T>) {
      using deserialize_t = decltype(&detail::remove_cv_ref_t<Serializer_T>::template deserialize_object<T>);
      using args_t = typename detail::examine_invocable<deserialize_t>::args;

      auto view = view_from_variadic(offset, (args_t *)nullptr);
      m_serializer.serialize_object(value, view);
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`T` cannot be serialized");
    }
  }

  template<std::size_t I>
  [[nodiscard]] constexpr static auto offset_for() noexcept -> std::size_t {
    using sizes_t = detail::integral_constant_tuple_t<detail::serialized_size<Ts, Serializer_T>()...>;
    using sizes_up_to_element_t = detail::variadic::clip_t<sizes_t, 0, I>;

    if constexpr (I == 0) {
      return 0;
    } else {
      return detail::variadic::sum_v<sizes_up_to_element_t>;
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
