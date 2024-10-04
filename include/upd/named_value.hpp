#pragma once

#include <array>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>
#include <array>
#include <ranges>
#include <string>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "description.hpp"
#include "detail/always_false.hpp"
#include "detail/integral_constant.hpp"
#include "detail/tuple_operations.hpp"
#include "detail/variadic/concat.hpp"
#include "integer.hpp"
#include "upd.hpp"
#include "tuple.hpp"
#include "description.hpp"
#include "detail/always_false.hpp"
#include "detail/integral_constant.hpp"
#include "detail/tuple_operations.hpp"
#include "detail/variadic/concat.hpp"
#include "detail/variadic/count.hpp"
#include "detail/variadic/find.hpp"
#include "integer.hpp"
#include "upd.hpp"

namespace upd::detail {

template<template<typename...> typename, typename>
struct instantiate_variadic;

template<template<typename...> typename TT, template<typename...> typename UT, typename... Ts>
struct instantiate_variadic<TT, UT<Ts...>> {
  using type = TT<Ts...>;
};

} // namespace upd::detail

namespace upd {

constexpr auto name_max_size = std::size_t{256};

template<typename T>
concept xinteger_instance = requires(T x) {
  { extended_integer{x} } -> std::same_as<T>;
};

template<typename T>
concept serializer = requires(
  T x,
  xuint<T::bytewidth> xui,
  xint<T::bytewidth - 1> xi,
  xuint<T::bytewidth> *it) {
  { x.serialize_unsigned(xui, it) } -> std::same_as<void>;
  { x.serialize_signed(xi, it) } -> std::same_as<void>;
  { x.deserialize_unsigned(std::as_const(it), width<decltype(xui)::bitsize>) } -> xinteger_instance;
  { x.deserialize_signed(std::as_const(it), width<decltype(xi)::bitsize>) } -> xinteger_instance;
};

template<serializer Serializer>
using byte_type = xuint<Serializer::bytewidth>;

struct name {
  template<std::size_t Size>
  consteval name(const char (&str)[Size]) noexcept(release): string{} {
    using namespace std::ranges;
    
    copy(str, string);
  }

  constexpr operator std::string_view() noexcept {
    return std::string_view{string};
  }

  char string[name_max_size];
};

template<name>
struct keyword;

[[nodiscard]] consteval inline auto operator==(const name &lhs, const name &rhs) noexcept(release) -> bool {
  return std::string_view{lhs.string} == std::string_view{rhs.string};
};

template<std::size_t N>
struct names {
  template<std::convertible_to<std::string_view>... Strings> requires (sizeof...(Strings) == N)
  consteval names(Strings... strs) noexcept(release): strings{} {
    using namespace std::ranges;
    auto i = std::size_t{0};
    (copy(std::string_view{strs}, strings[i++]), ...);
  }

  char strings[N][name_max_size];
};

template<std::convertible_to<std::string_view>... Strings>
names(Strings...) -> names<sizeof...(Strings)>;

template<name Identifier, typename T>
class named_value {
public:
  using value_type = T;

  constexpr static auto identifier = Identifier;

  template<typename... Args>
  constexpr explicit named_value(std::in_place_t, Args &&... args): m_value{UPD_FWD(args)...} {}

  template<typename F>
  [[nodiscard]] constexpr auto map(F &&f) & {
    decltype(auto) mapped_value = std::invoke(UPD_FWD(f), m_value);
    return keyword<identifier>{} = UPD_FWD(mapped_value);
  }

  template<typename F>
  [[nodiscard]] constexpr auto map(F &&f) const & {
    decltype(auto) mapped_value = std::invoke(UPD_FWD(f), m_value);
    return keyword<identifier>{} = UPD_FWD(mapped_value);
  }

  template<typename F>
  [[nodiscard]] constexpr auto map(F &&f) && {
    decltype(auto) mapped_value = std::invoke(UPD_FWD(f), std::move(m_value));
    return keyword<identifier>{} = UPD_FWD(mapped_value);
  }

  [[nodiscard]] constexpr auto value() & noexcept(release) -> T& {
    return m_value;
  }

  [[nodiscard]] constexpr auto value() const & noexcept(release) -> const T& {
    return m_value;
  }

  [[nodiscard]] constexpr auto value() && noexcept(release) -> T && {
    return std::move(m_value);
  }

  [[nodiscard]] constexpr auto value() const && noexcept(release) -> T && {
    return std::move(m_value);
  }

private:
  T m_value;
};

template<typename T>
concept named_value_instance = requires(T x) {
  { named_value{x} } -> std::same_as<T>;
};

template<names, typename... Ts>
[[nodiscard]] constexpr auto name_tuple(std::tuple<Ts...>) noexcept;

template<typename... Named_Tuple_Ts>
[[nodiscard]] constexpr auto concat_named_tuple(Named_Tuple_Ts &&...);

template<typename T>
concept variadic_instance = requires(T x) { 
  []<template<typename...> typename TT, typename... Ts>(const TT<Ts...> *) {} (&x);
};

template<template<typename...> typename TT, typename Args>
using instantiate_variadic = typename detail::instantiate_variadic<TT, Args>::type;

template<names Identifiers, typename... Ts>
requires (std::size(Identifiers.strings) == sizeof...(Ts))
class named_tuple {
  constexpr static auto element_types = zip(sequence<sizeof...(Ts)>, typelist<Ts...>{})
    .transform(unpack | []<typename T>(auto i, typebox<T>) {
        using type = named_value<Identifiers.strings[i.value], T>;
        return typebox<type>{};
    });

  using content_type = instantiate_variadic<tuple, std::remove_cvref_t<decltype(element_types)>>;

public:
  template<named_value_instance... NamedValues> requires (sizeof...(NamedValues) == sizeof...(Ts))
  constexpr explicit named_tuple(NamedValues && ...nvs): m_nvs{UPD_FWD(nvs)...} {}

  template<name Identifier>
  [[nodiscard]] constexpr auto get() & noexcept -> auto & {
    auto position = m_nvs
      .find_const([](auto nv) { return auto_constant<nv.identifier == Identifier>{}; });

    if constexpr (position < m_nvs.size()) {
      return m_nvs.at(position).value();
    } else {
      static_assert(UPD_ALWAYS_FALSE, "There are no element named `Identifier`");
    }
  }

  template<name Identifier>
  [[nodiscard]] constexpr auto get() const & noexcept -> const auto & {
    auto position = m_nvs
      .find_const([](auto nv) { return auto_constant<nv.identifier == Identifier>{}; });

    if constexpr (position < m_nvs.size()) {
      return m_nvs.at(position).value();
    } else {
      static_assert(UPD_ALWAYS_FALSE, "There are no element named `Identifier`");
    }
  }

  template<name Identifier>
  [[nodiscard]] constexpr auto get() && noexcept -> auto && {
    auto position = m_nvs
      .find_const([](auto nv) { return auto_constant<nv.identifier == Identifier>{}; });

    if constexpr (position < m_nvs.size()) {
      return std::move(m_nvs).at(position).value();
    } else {
      static_assert(UPD_ALWAYS_FALSE, "There are no element named `Identifier`");
    }
  }

  template<name Identifier>
  [[nodiscard]] constexpr auto get() const && noexcept -> const auto && {
    auto position = m_nvs
      .find_const([](auto nv) { return auto_constant<nv.identifier == Identifier>{}; });

    if constexpr (position < m_nvs.size()) {
      return std::move(m_nvs).at(position).value();
    } else {
      static_assert(UPD_ALWAYS_FALSE, "There are no element named `Identifier`");
    }
  }

  template<name Identifier>
  [[nodiscard]] constexpr auto operator[](keyword<Identifier>) & noexcept(release) -> auto & {
    return get<Identifier>();
  }

  template<name Identifier>
  [[nodiscard]] constexpr auto operator[](keyword<Identifier>) const & noexcept(release) -> const auto & {
    return get<Identifier>();
  }

  template<name Identifier>
  [[nodiscard]] constexpr auto operator[](keyword<Identifier>) && noexcept(release) -> auto && {
    return std::move(*this).template get<Identifier>();
  }

  template<name Identifier>
  [[nodiscard]] constexpr auto operator[](keyword<Identifier>) const && noexcept(release) -> const auto && {
    return std::move(*this).template get<Identifier>();
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr void serialize(Serializer &ser, OutputIt output) const {
    auto serialize_pack = [&](const auto &...nvs) { (nvs.value().serialize(ser, output), ...); };
    m_nvs.apply(serialize_pack);
  }

private:
  content_type m_nvs;
};

template<named_value_instance... NamedValues>
explicit named_tuple(NamedValues...) -> named_tuple<{NamedValues::identifier...}, typename NamedValues::value_type...>;

template<typename T>
concept named_tuple_instance = requires(T x) {
  { named_tuple{x} } -> std::same_as<T>;
};


template<named_tuple_instance... NamedTuples>
[[nodiscard]] constexpr auto concat(NamedTuples &&...nts) {
  return tuple<NamedTuples &&...>{nts...}
    .flatten()
    .apply([](auto &&... nvs) { return named_tuple{UPD_FWD(nvs)...}; });
}

template<named_value_instance Lhs, named_value_instance Rhs>
[[nodiscard]] constexpr auto operator,(Lhs &&lhs, Rhs &&rhs) noexcept(release) {
  return named_tuple{UPD_FWD(lhs), UPD_FWD(rhs)};
}

template<named_tuple_instance Lhs, named_value_instance Rhs>
[[nodiscard]] constexpr auto operator,(Lhs &&lhs, Rhs &&rhs) noexcept(release) {
  auto singleton = named_tuple{UPD_FWD(rhs)};
  return concat(UPD_FWD(lhs), std::move(singleton));
}

template<name Identifier>
struct keyword {
  constexpr static auto identifier = Identifier;

  template<typename T>
  [[nodiscard]] constexpr auto operator=(T x) const noexcept(release) -> named_value<identifier, T> {
    return named_value<Identifier, T>{std::in_place, UPD_FWD(x)};
  }
};

} // namespace upd

namespace upd::literals {

template<name Identifier>
[[nodiscard]] constexpr auto operator""_kw() noexcept(release) {
  return keyword<Identifier>{};
}

} // namespace upd::literals
