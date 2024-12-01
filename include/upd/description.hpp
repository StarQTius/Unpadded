#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <tuple>
#include <type_traits>
#include <variant>
#include <expected>
#include <iterator>

#include "detail/always_false.hpp"
#include "detail/has_value_member.hpp"
#include "detail/integral_constant.hpp"
#include "detail/variadic/equals.hpp"
#include "detail/variadic/filter.hpp"
#include "detail/variadic/map.hpp"
#include "detail/variadic/product.hpp"
#include "detail/variadic/to_array.hpp"
#include "integer.hpp"
#include "named_value.hpp"
#include "token.hpp"
#include "tuple.hpp"
#include "upd.hpp"

#define UPD_WELL_FORMED(...) \
  do { \
    if (requires { __VA_ARGS__; }) { \
      __VA_ARGS__; \
    } \
  } while (false)

namespace upd::detail {

template<std::input_or_output_iterator Iter>
class iterator_reference {
public:
  using iterator_type = Iter;
  using difference_type = typename std::iterator_traits<Iter>::difference_type;
  using value_type = typename std::iterator_traits<Iter>::value_type;
  using pointer = typename std::iterator_traits<Iter>::pointer;
  using reference = typename std::iterator_traits<Iter>::reference;
  using iterator_category = std::input_iterator_tag;
  
  constexpr explicit iterator_reference(iterator_type &iter) : m_iter{iter}, m_cache{*iter} {
    ++iter;
  }

  [[nodiscard]] constexpr auto operator*() const -> value_type { 
    return m_cache;
  }

  constexpr auto operator++() -> iterator_reference & {
    m_cache = *m_iter;
    ++m_iter.get();
    return *this;
  }

  constexpr auto operator++(int) -> iterator_reference {
    auto retval = *this;
    m_cache = *m_iter;
    ++m_iter.get();
    
    return retval;
  }

private:
  std::reference_wrapper<iterator_type> m_iter;
  value_type m_cache;
};

} // namespace upd::detail

namespace upd {

struct no_error {};

struct not_matching_deduction {
  const char *identifier;
};

using error_data_types = typelist<
  no_error,
  not_matching_deduction
>;

template<typename T>
concept variadic_instance = requires(T x) {
  []<template<typename ...> typename TT, typename ...Ts>(const TT<Ts...> &) {} (x);
};

template<typename T>
using is_variadic_instance = auto_constant<variadic_instance<T>>;

template<typename T>
concept value_variadic_instance = requires(T x) {
  []<template<auto ...> typename TT, auto ...Values>(const TT<Values...> &) {} (x);
};

template<typename T>
using is_value_variadic_instance = auto_constant<value_variadic_instance<T>>;

template<typename T, typename Variadic>
concept element_of = variadic_instance<Variadic>
&& instantiate_variadic<detail::lite_tuple, Variadic>::has_type(typebox<T>{});

template<typename T, variadic_instance Variadic>
using is_element_of = auto_constant<element_of<T, Variadic>>;

template<auto Value, typename Variadic>
concept value_element_of = value_variadic_instance<Variadic>
&& instantiate_variadic<detail::lite_tuple, Variadic>::has_type(expr<Value>);

template<auto Value, value_variadic_instance Variadic>
using is_value_element_of = auto_constant<value_element_of<Value, Variadic>>;

template<typename T>
concept error_data = element_of<std::remove_cvref_t<T>, error_data_types>;

class error {
  using data_type = instantiate_variadic<std::variant, error_data_types>;

  public:
    constexpr error() noexcept(release) = default;
    constexpr error(const error&) noexcept(release) = default;
    constexpr error(error&&) noexcept(release) = default;

    template<error_data ErrorData>
    constexpr error(ErrorData &&err_data) noexcept(release) : m_data{UPD_FWD(err_data)} {}

    constexpr auto operator=(const error&) noexcept(release) -> error & = default;
    constexpr auto operator=(error&&) noexcept(release) -> error & = default;

    template<error_data ErrorData>
    constexpr auto operator=(ErrorData &&err_data) noexcept(release) -> error& {
      m_data = UPD_FWD(err_data);
      return *this;
    }

    [[nodiscard]] constexpr operator bool() const noexcept(release) {
      return !std::holds_alternative<no_error>(m_data);
    }

    template<typename F>
    constexpr auto visit(F &&f) const -> decltype(auto) {
      return std::visit(UPD_FWD(f), m_data);
    }

  private:
    data_type m_data;
};

template<typename T>
using result = std::expected<T, error>;

template<typename T>
[[nodiscard]] constexpr auto result_if_no_error(T &&x, const error &err) -> result<std::remove_cvref_t<T>> {
  if (err) {
    return std::unexpected{err};
  } else {
    return UPD_FWD(x);
  }
}

template<typename T>
[[nodiscard]] constexpr auto result_if_no_error(T &&x, error &&err) -> result<std::remove_cvref_t<T>> {
  if (err) {
    return std::unexpected{std::move(err)};
  } else {
    return UPD_FWD(x);
  }
}

template<typename>
class invoker_iterator;

template<typename Parent>
requires instance_of<std::remove_cv_t<Parent>, invoker_iterator>
class invoker_iterator_proxy;

template<typename F>
class invoker_iterator {
  template<typename Parent>
  requires instance_of<std::remove_cv_t<Parent>, invoker_iterator>
  friend class invoker_iterator_proxy;

public:
  using invocable_type = F;

  constexpr explicit invoker_iterator(F f) noexcept : m_f{std::move(f)} {}

  constexpr auto operator*() noexcept -> invoker_iterator_proxy<invoker_iterator> {
    return invoker_iterator_proxy{this};
  }

  constexpr auto operator*() const noexcept -> invoker_iterator_proxy<const invoker_iterator> {
    return invoker_iterator_proxy{this};
  }

  constexpr auto operator++() noexcept -> invoker_iterator & {
    return *this;
  }

  constexpr auto operator++() const noexcept -> const invoker_iterator & {
    return *this;
  }

private:
  F m_f;
};

template<typename Parent>
requires instance_of<std::remove_cv_t<Parent>, invoker_iterator>
class invoker_iterator_proxy {
  template<typename>
  friend class invoker_iterator;

public:
  using invocable_type = typename Parent::invocable_type;

  template<typename T> requires invocable<invocable_type, T>
  constexpr auto operator=(T &&x) const -> const invoker_iterator_proxy & {
    std::invoke(m_parent->m_f, UPD_FWD(x));
    return *this;
  }

private:
  constexpr invoker_iterator_proxy(Parent *parent) noexcept: m_parent{parent} {}

  Parent *m_parent;
};

template<typename Iter, typename F>
class transformer_iterator {
public:
  using difference_type = typename std::iterator_traits<Iter>::difference_type;
  using value_type = std::remove_cvref_t<std::invoke_result_t<F, typename std::iterator_traits<Iter>::value_type>>;
  using reference = std::invoke_result_t<F, typename std::iterator_traits<Iter>::value_type> &;
  using pointer = decltype(&std::declval<reference>());
  using iterator_category = std::input_iterator_tag;

  constexpr explicit transformer_iterator(Iter iter, F f) noexcept:
    m_iter{iter},
    m_f{std::move(f)}
  {}

  [[nodiscard]] constexpr auto operator*() -> decltype(auto) {
    return m_f(*m_iter);
  }

  constexpr auto operator++() -> transformer_iterator & {
    ++m_iter;
    return *this;
  }

private:
  Iter m_iter;
  F m_f;
};

} // namespace upd

namespace upd::descriptor {

struct all_fields_t {};

constexpr auto all_fields = all_fields_t{};

template<name, bool Is_Signed, std::size_t Width>
constexpr auto field(signedness_t<Is_Signed>, width_t<Width>) noexcept(release);

template<name, typename T, std::size_t Width>
constexpr auto constant(T, width_t<Width>) noexcept(release);

} // namespace upd::descriptor

namespace upd {

template<typename Range>
[[nodiscard]] constexpr auto all_of(const Range &range) noexcept -> bool {
  for (const auto &e : range) {
    if (!e) {
      return false;
    }
  }

  return true;
}

enum class field_tag {
  pure_field,
  constant,
  checksum,
};

template<named_value_instance ...NamedValues>
using named_value_bundle = decltype(named_tuple{std::declval<NamedValues>()...});

template<typename T>
concept field_like = requires(T) {
  typename T::value_type;
} && requires(T x) {
  { x.default_value() } -> std::same_as<typename T::value_type>;
};

template<typename T, typename Serializer>
concept deducable_field = field_like<T>
&& serializer<Serializer>
&& requires(
    T x,
    Serializer ser) {
  { x.deduce(named_tuple{}, ser) } -> std::same_as<typename T::value_type>;
};

template<typename T, typename Serializer>
concept decodable_field = serializer<Serializer>
&& deducable_field<T, Serializer>
&& requires(
    T x,
    Serializer ser,
    const byte_type<Serializer> *src) {
  { x.decode(src, ser) } -> std::same_as<typename T::value_type>;
};

template<typename T, typename Serializer>
concept encodable_field = serializer<Serializer>
&& deducable_field<T, Serializer>
&& requires(
    T x,
    Serializer ser,
    typename T::value_type value,
    byte_type<Serializer> *dest) {
  { x.encode(value, ser, dest) } -> std::same_as<void>;
};

template<field_like... Ts>
class description {
  template<typename... _Ts, typename... Us>
  friend constexpr auto operator|(description<_Ts...> lhs, description<Us...> rhs) noexcept(release);

public:
  constexpr static auto identifiers = typelist<Ts...>{}
    .transform([]<typename T>(typebox<T>) { return expr<T::identifier>; });

  explicit constexpr description(Ts ...fields) : m_fields{std::move(fields)...} {}

  template<named_tuple_instance NamedTuple, serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  requires (encodable_field<Ts, Serializer> && ...)
  constexpr void encode(const NamedTuple &nargs, Serializer &ser, OutputIt dest) const {
    auto packet = m_fields
      .transform([&]<typename Field>(const Field &field) {
        using value_type = typename Field::value_type;
        auto id = expr<field.identifier>;
        if constexpr (nargs.contains(id)) {
          return keyword<field.identifier>{} = value_type{nargs[id]};
        } else {
          return keyword<field.identifier>{} = field.default_value();
        }
      })
      .apply([](auto &&... field_values) { return named_tuple{UPD_FWD(field_values)...}; });

    zip(packet, m_fields)
      .for_each(unpack | [&](auto &field_value, const auto &field) {
        UPD_WELL_FORMED(field_value.value() = field.deduce(std::as_const(packet), ser));
      });

    zip(packet, m_fields)
      .for_each(unpack | [&](const auto &field_value, const auto &field) {
        field.encode(field_value.value(), ser, dest);
      });
  }

  template<serializer Serializer, std::input_iterator InputIt>
  requires (decodable_field<Ts, Serializer> && ...)
  [[nodiscard]] constexpr auto decode(InputIt src, Serializer &ser) const {
    auto err = error{};
    auto retval = m_fields
      .apply([](const auto & ...fields) {
          return named_tuple { (keyword<fields.identifier>{} = fields.default_value())... };
      });

    m_fields
      .for_each([&](const auto &field) {
        retval[expr<field.identifier>] = field.decode(detail::iterator_reference{src}, ser);
      });

    zip(retval, m_fields)
      .for_each(unpack | [&](const auto &field_value, const auto &field) {
        if (!err && field_value.value() != field.deduce(retval, ser)) {
          err = not_matching_deduction{field_value.identifier.string};
        }
      });

    return result_if_no_error(std::move(retval), std::move(err));
  }

private:
  tuple<Ts...> m_fields;
};

template<typename... Ts, typename... Us>
[[nodiscard]] constexpr auto operator|(description<Ts...> lhs, description<Us...> rhs) noexcept(release) {
  auto concatenated_fields = std::move(lhs.m_fields) + std::move(rhs.m_fields);

  // For each identifier of the merged description, we count how often it
  // appears. If the total is not equal to the number of fields, we know that
  // there are duplicate identifiers.
  auto self_comparison_count = concatenated_fields
    .type_only()
    .transform([]<typename T>(typebox<T>) { return expr<T::identifier>; })
    .square()
    .transform(unpack | equal_to)
    .fold_left(expr<0uz>, plus);

  static_assert(self_comparison_count == concatenated_fields.size(), "Merging these descriptions would result in duplicate IDs");

  return std::move(concatenated_fields).apply(
    [](auto && ...fields) { return description{std::move(fields)...}; }
  );
}

} // namespace upd

namespace upd::descriptor {

template<name Identifier, bool Signedness, std::size_t Width>
struct field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, xint<width>, xuint<width>>;

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type {
    return value_type{};
  }

  template<named_tuple_like Packet, serializer Serializer>
  [[nodiscard]] constexpr static auto deduce(const Packet &packet, Serializer &) -> value_type {
    return packet.at(expr<identifier>);
  }

  template<serializer Serializer, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, Serializer &ser) -> value_type {
    if constexpr (is_signed) {
      return ser.deserialize_signed(src, upd::width<width>);
    } else {
      return ser.deserialize_unsigned(src, upd::width<width>);
    }
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr static void encode(value_type value, Serializer &ser, OutputIt dest) {
    if constexpr (is_signed) {
      return ser.serialize_signed(value, dest);
    } else {
      return ser.serialize_unsigned(value, dest);
    }
  }
};

template<name Identifier, bool Signedness, std::size_t Width>
[[nodiscard]] constexpr auto field(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
  field_like auto retval = field_t<Identifier, Signedness, Width>{};

  return description{retval};
}

template<name Identifier, std::size_t Width>
struct constant_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = xuint<width>;

  value_type field_value;

  [[nodiscard]] constexpr auto default_value() const noexcept(release) -> value_type {
    return field_value;
  }

  template<named_tuple_like Packet, serializer Serializer>
  [[nodiscard]] constexpr static auto deduce(const Packet &packet, Serializer &) -> value_type {
    return packet.at(expr<identifier>);
  }

  template<serializer Serializer, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, Serializer &ser) -> value_type {
    return ser.deserialize_unsigned(src, upd::width<width>);
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr static void encode(value_type value, Serializer &ser, OutputIt dest) {
    return ser.serialize_unsigned(value, dest);
  }
};

template<name Identifier, typename T, std::size_t Width>
[[nodiscard]] constexpr auto constant(T n, width_t<Width>) noexcept(release) {
  auto retval = constant_t<Identifier, Width>{n};
  return description{retval};
}

template<name Identifier, typename BinaryOp, std::size_t Width, typename FieldFilter>
struct checksum_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = xuint<width>;

  BinaryOp op;
  value_type init;
  FieldFilter identifier_filter;

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type {
    return 0u;
  }

  template<named_tuple_like Packet, serializer Serializer>
  [[nodiscard]] constexpr auto deduce(const Packet &packet, Serializer &ser) const -> value_type {
    auto field_filter = [&](const auto &nv) {
      return UPD_INVOKE(identifier_filter, expr<nv.identifier>);
    };

    auto retval = init;
    auto it = invoker_iterator {[&](auto byte) { retval = UPD_INVOKE(op, retval, byte); }};

    packet
      .filter(field_filter)
      .transform([](const auto &named_field) -> const auto & { return named_field.value(); })
      .for_each([&](const auto &field_value) { field_value.serialize(ser, it); });
  
    return retval;
  }

  template<serializer Serializer, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, Serializer &ser) -> value_type {
    return ser.deserialize_unsigned(src, upd::width<width>);
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr static void encode(value_type value, Serializer &ser, OutputIt dest) {
    return ser.serialize_unsigned(value, dest);
  }
};

template<name Identifier, typename BinaryOp, std::size_t Width>
[[nodiscard]] constexpr auto checksum(
    BinaryOp op,
    xuint<Width> init,
    all_fields_t
) noexcept(release) {
  auto is_not_this_field = [](auto id) {
    return expr<id != Identifier>;
  };

  auto retval = checksum_t<
    Identifier,
    BinaryOp,
    Width,
    decltype(is_not_this_field)>
  {std::move(op), init, is_not_this_field};
  
  return description{std::move(retval)};
}

} // namespace upd::descriptor

namespace upd::literals {

[[nodiscard]] constexpr inline auto operator""_h(const char *str, std::size_t size) noexcept -> std::size_t {
  constexpr auto numlim = std::numeric_limits<char>{};
  constexpr auto min = std::intmax_t{numlim.min()};
  constexpr auto max = std::intmax_t{numlim.max()};

  auto retval = std::size_t{0};

  // `std::hash` cannot be invoked in constant expression, so here is the poor man's hashing function for the moment
  for (std::size_t i = 0; i < size; ++i) {
    retval += str[i] - min;
    retval *= max - min;
  }

  return retval;
}

} // namespace upd::literals
