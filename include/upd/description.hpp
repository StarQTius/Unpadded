#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <iterator>
#include <limits>
#include <tuple>
#include <type_traits>
#include <variant>

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
#include "static_vector.hpp"
#include "token.hpp"
#include "tuple.hpp"
#include "upd.hpp"

#define UPD_WELL_FORMED(...)                                                                                           \
  do {                                                                                                                 \
    if (requires { __VA_ARGS__; }) {                                                                                   \
      __VA_ARGS__;                                                                                                     \
    }                                                                                                                  \
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

  constexpr iterator_reference(iterator_type &iter, bool read) : m_iter{iter}, m_read{read} {}
  constexpr iterator_reference(const iterator_reference &other) : m_iter{other.m_iter}, m_read{false} {}

  [[nodiscard]] constexpr auto operator*() const -> value_type {
    m_read = true;
    return *m_iter.get();
  }

  constexpr auto operator++() -> iterator_reference & {
    ++m_iter.get();
    return *this;
  }

  constexpr auto operator++(int) -> iterator_reference {
    auto retval = *this;
    ++m_iter.get();
    return retval;
  }

  ~iterator_reference() {
    if (m_read) {
      ++m_iter.get();
    }
  }

  std::reference_wrapper<iterator_type> m_iter;
  mutable bool m_read;
};

template<typename Iter>
[[nodiscard]] constexpr auto make_itererator_reference(Iter &iter) -> decltype(auto) {
  if constexpr (is_instance_of<Iter, iterator_reference>()) {
    return iter;
  } else {
    return iterator_reference<Iter>{iter, false};
  }
}

} // namespace upd::detail

namespace upd {

template<typename Enum, std::size_t Width>
struct xenum {
  using enum_type = Enum;
  constexpr static auto width = Width;

  using underlying_type = std::underlying_type_t<enum_type>;
  constexpr static auto is_signed = std::is_signed_v<underlying_type>;

  constexpr xenum(enum_type val) noexcept(release) : value{val} {}

  constexpr explicit xenum(extended_integer<width, underlying_type> xn) noexcept(release)
      : value{static_cast<enum_type>(xn.value())} {}

  constexpr explicit operator extended_integer<width, underlying_type>() const noexcept(release) { return to_xint(); }

  constexpr operator enum_type() const noexcept(release) { return value; }

  [[nodiscard]] constexpr auto to_xint() const noexcept(release) {
    return extended_integer<width, underlying_type>{std::in_place, static_cast<underlying_type>(value)};
  }

  enum_type value;
};

template<auto Code, typename... Args>
struct choice_t {
  constexpr static auto code = Code;
  constexpr static auto argument_types = typelist<Args...>{};

  tuple<Args...> arguments;
};

template<auto Code, typename... Args>
[[nodiscard]] constexpr auto choice(Args &&...args) -> choice_t<Code, Args...> {
  return choice_t<Code, Args...>{.arguments = {UPD_FWD(args)...}};
}

template<typename Tuple>
struct equivalent_typelist {
  using type = decltype(
    sequence<std::tuple_size_v<Tuple>>
      .transform([](auto i) { return typebox<std::tuple_element_t<i, Tuple>>{}; })
      .to_typelist()
  );
};

template<typename Tuple>
using equivalent_typelist_t = typename equivalent_typelist<Tuple>::type;

template<typename T>
concept inversible = requires(T x) {
  inverse(UPD_FWD(x));
  { inverse(inverse(UPD_FWD(x))) } -> std::convertible_to<T>;
};

template<typename T>
struct add_some {
  T offset;

  template<typename U>
  [[nodiscard]] constexpr auto operator()(U &&x) const noexcept(release) {
    return UPD_FWD(x) + offset;
  }
};

template<typename T>
struct substract_some {
  T offset;

  template<typename U>
  [[nodiscard]] constexpr auto operator()(U &&x) const noexcept(release) {
    return UPD_FWD(x) - offset;
  }
};

template<typename T>
struct multiply_some {
  T factor;

  template<typename U>
  [[nodiscard]] constexpr auto operator()(U &&x) const noexcept(release) {
    return UPD_FWD(x) * factor;
  }
};

template<typename T>
struct divide_some {
  T factor;

  template<typename U>
  [[nodiscard]] constexpr auto operator()(U &&x) const noexcept(release) {
    return UPD_FWD(x) / factor;
  }
};

template<inversible... Inversibles>
struct bijective_chain {
  tuple<Inversibles...> operations;

  template<typename Self, typename U>
  [[nodiscard]] constexpr auto operator()(this Self &&self, U x) {
    return UPD_FWD(self).operations.fold_left(x, [](auto acc, auto &&op) { return UPD_INVOKE(op, acc); });
  };

  template<typename Self, inversible Inversible>
  [[nodiscard]] constexpr auto and_then(this Self &&self, Inversible &&op) {
    return UPD_FWD(self).operations.apply(
        [&](auto &&...ops) { return bijective_chain<Inversibles..., Inversible>{{UPD_FWD(ops)..., UPD_FWD(op)}}; });
  }
};

template<typename T>
[[nodiscard]] constexpr auto inverse(add_some<T> op) noexcept(release) -> substract_some<T> {
  return substract_some{op.offse};
}

template<typename T>
[[nodiscard]] constexpr auto inverse(substract_some<T> op) noexcept(release) -> add_some<T> {
  return add_some{op.offset};
}

template<typename T>
[[nodiscard]] constexpr auto inverse(multiply_some<T> op) noexcept(release) -> divide_some<T> {
  return divide_some{op.factor};
}

template<typename T>
[[nodiscard]] constexpr auto inverse(divide_some<T> op) noexcept(release) -> multiply_some<T> {
  return multiply_some{op.factor};
}

template<inversible... Inversibles>
[[nodiscard]] constexpr auto inverse(const bijective_chain<Inversibles...> &chain) {
  auto inv_ops = chain.operations.transform([](const auto &op) { return inverse(op); }).reverse();

  return bijective_chain{std::move(inv_ops)};
}

template<inversible... Inversibles>
[[nodiscard]] constexpr auto inverse(bijective_chain<Inversibles...> &&chain) {
  auto inv_ops = std::move(chain).operations.transform([](auto &&op) { return inverse(std::move(op)); }).reverse();

  return bijective_chain{std::move(inv_ops)};
}

template<auto Identifier, typename F, inversible... Inversibles>
struct field_expression_t {
  constexpr static auto from_identifier = Identifier;

  auto_constant<Identifier> id_const;
  F get_value;
  bijective_chain<Inversibles...> chain;

  template<typename Self, typename T>
  [[nodiscard]] constexpr auto operator+(this Self &&self, T &&x) {
    return UPD_FWD(self).and_then(add_some{UPD_FWD(x)});
  }

  template<typename Self, typename T>
  [[nodiscard]] constexpr auto operator-(this Self &&self, T &&x) {
    return UPD_FWD(self).and_then(substract_some{UPD_FWD(x)});
  }

  template<typename Self, typename T>
  [[nodiscard]] constexpr auto operator*(this Self &&self, T &&x) {
    return UPD_FWD(self).and_then(multiply_some{UPD_FWD(x)});
  }

  template<typename Self, typename T>
  [[nodiscard]] constexpr auto operator/(this Self &&self, T &&x) {
    return UPD_FWD(self).and_then(divide_some{UPD_FWD(x)});
  }

  template<typename Self, typename T>
  [[nodiscard]] constexpr auto deduce(this Self &&self, T &&from) {
    decltype(auto) value = UPD_INVOKE(UPD_FWD(self).get_value, UPD_FWD(from));
    return UPD_INVOKE(UPD_FWD(self).chain, UPD_FWD(value));
  }

  template<typename Self, inversible Inversible>
  [[nodiscard]] constexpr auto and_then(this Self &&self, Inversible &&op) {
    return field_expression_t<Identifier, F, Inversibles..., Inversible>{
        self.id_const, UPD_FWD(self).get_value, UPD_FWD(self).chain.and_then(UPD_FWD(op))};
  }
};

[[nodiscard]] constexpr auto bitsize(std::monostate) noexcept(release) -> std::size_t { return 0; }

template<typename... Ts>
[[nodiscard]] constexpr auto bitsize(const std::variant<Ts...> &sum_of_field_values) noexcept(release) -> std::size_t {
  return std::visit([](const auto &field_value) { return bitsize(field_value); }, sum_of_field_values);
}

template<names Identifiers, typename... Ts>
[[nodiscard]] constexpr auto bitsize(const named_tuple<Identifiers, Ts...> &named_field_values) noexcept(release)
    -> std::size_t {
  return named_field_values.fold_left(
      0uz, [](std::size_t acc, const auto &field_value) { return acc + bitsize(field_value.value()); });
}

template<std::size_t Bitsize, typename Underlying>
[[nodiscard]] constexpr static auto bitsize(xinteger<Bitsize, Underlying> xi) -> std::size_t {
  return xi.bitsize;
}

template<typename Enum, std::size_t Bitsize>
[[nodiscard]] constexpr static auto bitsize(xenum<Enum, Bitsize> xe) -> std::size_t {
  return xe.width;
}

template<typename T, std::size_t Max>
[[nodiscard]] constexpr static auto bitsize(const static_vector<T, Max> &svec) -> std::size_t {
  namespace stdr = std::ranges;
  return stdr::fold_left(svec, 0uz, [](auto acc, const auto &elem) { return acc + bitsize(elem); });
}

template<name Identifier>
constexpr auto value_of =
    field_expression_t{expr<Identifier>, [](auto &packet) -> auto & { return packet[expr<Identifier>]; }};

template<name Identifier>
constexpr auto length_of = field_expression_t{expr<Identifier>, [](const auto &packet) {
                                                return extended_integer<16, std::size_t>{
                                                    static_cast<std::uint16_t>(bitsize(packet[expr<Identifier>]))};
                                              }};

template<name Identifier>
constexpr auto code_of =
    field_expression_t{expr<Identifier>, [](const auto &packet) { return packet[expr<Identifier>].index(); }};

template<auto Match, typename Result>
struct when_then_t {
  constexpr static auto match = Match;

  using result_type = Result;

  result_type result;
};

template<auto Match>
struct when_t {
  constexpr static auto match = Match;

  template<typename Result>
  [[nodiscard]] constexpr auto operator=(Result &&result) const {
    using result_type = std::decay_t<Result>;
    return when_then_t<match, result_type>{UPD_FWD(result)};
  }
};

template<auto Match>
constexpr auto when = when_t<Match>{};

template<typename... WhenThens>
[[nodiscard]] constexpr auto aggregate_when_thens(WhenThens &&...when_thens) {
  return tagged_tuple{
      named_value<when_thens.match, typename WhenThens::result_type>{std::in_place, UPD_FWD(when_thens).result}...};
}

struct no_error {};

struct not_matching_deduction {
  const char *identifier;
};

struct invalid_code_in_one_of {
  const char *identifier;
  std::intmax_t code;
};

struct negative_repetition_count {
  const char *identifier;
  std::intmax_t count;
};

struct repeated_beyond_max {
  const char *identifier;
  std::uintmax_t count;
  std::size_t max;
};

using error_data_types =
    typelist<no_error, not_matching_deduction, invalid_code_in_one_of, negative_repetition_count, repeated_beyond_max>;

template<typename T>
concept variadic_instance =
    requires(T x) { []<template<typename...> typename TT, typename... Ts>(const TT<Ts...> &) {}(x); };

template<typename T>
using is_variadic_instance = auto_constant<variadic_instance<T>>;

template<typename T>
concept value_variadic_instance =
    requires(T x) { []<template<auto...> typename TT, auto... Values>(const TT<Values...> &) {}(x); };

template<typename T>
using is_value_variadic_instance = auto_constant<value_variadic_instance<T>>;

template<typename T, typename Variadic>
concept element_of =
    variadic_instance<Variadic> && instantiate_variadic<detail::lite_tuple, Variadic>::has_type(typebox<T>{});

template<typename T, variadic_instance Variadic>
using is_element_of = auto_constant<element_of<T, Variadic>>;

template<auto Value, typename Variadic>
concept value_element_of =
    value_variadic_instance<Variadic> && instantiate_variadic<detail::lite_tuple, Variadic>::has_type(expr<Value>);

template<auto Value, value_variadic_instance Variadic>
using is_value_element_of = auto_constant<value_element_of<Value, Variadic>>;

template<typename T>
concept error_data = element_of<std::remove_cvref_t<T>, error_data_types>;

class error {
  using data_type = instantiate_variadic<std::variant, error_data_types>;

public:
  constexpr error() noexcept(release) = default;
  constexpr error(const error &) noexcept(release) = default;
  constexpr error(error &&) noexcept(release) = default;

  template<error_data ErrorData>
  constexpr error(ErrorData &&err_data) noexcept(release) : m_data{UPD_FWD(err_data)} {}

  constexpr auto operator=(const error &) noexcept(release) -> error & = default;
  constexpr auto operator=(error &&) noexcept(release) -> error & = default;

  template<error_data ErrorData>
  constexpr auto operator=(ErrorData &&err_data) noexcept(release) -> error & {
    m_data = UPD_FWD(err_data);
    return *this;
  }

  [[nodiscard]] constexpr operator bool() const noexcept(release) { return !std::holds_alternative<no_error>(m_data); }

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
  using difference_type = std::ptrdiff_t;

  constexpr explicit invoker_iterator(F *f) noexcept : m_f{f} {}

  constexpr auto operator*() noexcept -> invoker_iterator_proxy<invoker_iterator> {
    return invoker_iterator_proxy{this};
  }

  constexpr auto operator*() const noexcept -> invoker_iterator_proxy<const invoker_iterator> {
    return invoker_iterator_proxy{this};
  }

  constexpr auto operator++() noexcept -> invoker_iterator & { return *this; }

  constexpr auto operator++() const noexcept -> const invoker_iterator & { return *this; }

  constexpr auto operator++(int) noexcept -> invoker_iterator & { return *this; }

  constexpr auto operator++(int) const noexcept -> const invoker_iterator & { return *this; }

private:
  F *m_f;
};

template<typename Parent>
  requires instance_of<std::remove_cv_t<Parent>, invoker_iterator>
class invoker_iterator_proxy {
  template<typename>
  friend class invoker_iterator;

public:
  using invocable_type = typename Parent::invocable_type;

  template<typename T>
    requires invocable<invocable_type, T>
  constexpr auto operator=(T &&x) const -> const invoker_iterator_proxy & {
    UPD_INVOKE(*m_parent->m_f, UPD_FWD(x));
    return *this;
  }

private:
  constexpr invoker_iterator_proxy(Parent *parent) noexcept : m_parent{parent} {}

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

  constexpr explicit transformer_iterator(Iter iter, F f) noexcept : m_iter{iter}, m_f{std::move(f)} {}

  [[nodiscard]] constexpr auto operator*() -> decltype(auto) { return m_f(*m_iter); }

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

template<named_value_instance... NamedValues>
using named_value_bundle = decltype(named_tuple{std::declval<NamedValues>()...});

template<typename T>
concept field_like = requires(T) { typename T::value_type; } && requires(T x) {
  { x.default_value() } -> std::same_as<typename T::value_type>;
};

template<typename T, typename Serializer>
concept deducible_field =
    field_like<T> && serializer<Serializer> && requires(T x, Serializer ser, named_tuple<{}> packet) {
      { x.deduce(packet, ser) } -> std::same_as<void>;
    };

template<typename T, typename Serializer>
concept decodable_field =
    serializer<Serializer> && deducible_field<T, Serializer> &&
    requires(T x, Serializer ser, const named_tuple<{}> packet, const byte_type<Serializer> *src) {
      { x.decode(src, packet, ser) } -> std::same_as<typename T::value_type>;
    };

template<typename T, typename Serializer>
concept encodable_field = serializer<Serializer> && deducible_field<T, Serializer> &&
                          requires(T x, Serializer ser, typename T::value_type value, byte_type<Serializer> *dest) {
                            { x.encode(value, ser, dest) } -> std::same_as<void>;
                          };

template<field_like... Ts>
class description {
  template<typename... _Ts, typename... Us>
  friend constexpr auto operator|(description<_Ts...> lhs, description<Us...> rhs) noexcept(release);

public:
  constexpr static auto identifiers =
      typelist<Ts...>{}.transform([]<typename T>(typebox<T>) { return expr<T::identifier>; }).apply([](auto... ids) {
        return names{ids...};
      });

  using result_type = named_tuple<identifiers, typename Ts::value_type...>;

  explicit constexpr description(Ts... fields) : m_fields{std::move(fields)...} {}

  template<named_tuple_instance NamedTuple, serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  // requires (encodable_field<Ts, Serializer> && ...)
  constexpr void encode(const NamedTuple &nargs, Serializer &ser, OutputIt dest) const {
    auto packet = m_fields
                      .transform([&]<typename Field>(const Field &field) {
                        auto id = expr<field.identifier>;
                        if constexpr (nargs.contains(id)) {
                          return keyword<field.identifier>{} = field.make_value(nargs[id]);
                        } else {
                          return keyword<field.identifier>{} = field.default_value();
                        }
                      })
                      .apply([](auto &&...field_values) { return named_tuple{UPD_FWD(field_values)...}; });

    m_fields.for_each([&](const auto &field) {
      field.deduce(packet, ser, m_fields).for_each([&](const auto &named_value) {
        packet[expr<named_value.identifier>] = UPD_FWD(named_value).value();
      });
    });

    m_fields.for_each([&](const auto &field) {
      ser.checkpoint(field.identifier.string);
      field.encode(packet[expr<field.identifier>], ser, dest);
    });
  }

  template<serializer Serializer, std::input_iterator InputIt>
  // requires (decodable_field<Ts, Serializer> && ...)
  [[nodiscard]] constexpr auto decode(InputIt src, Serializer &ser) const {
    return decode(src, named_tuple{}, ser);
  }

  template<serializer Serializer, typename Packet, std::input_iterator InputIt>
  // requires (decodable_field<Ts, Serializer> && ...)
  [[nodiscard]] constexpr auto decode(InputIt src, const Packet &ctx, Serializer &ser) const {
    auto err = error{};
    auto retval = m_fields.apply(
        [](const auto &...fields) { return named_tuple{(keyword<fields.identifier>{} = fields.default_value())...}; });

    if (!err) {
      m_fields.for_each([&](const auto &field) {
        ser.checkpoint(field.identifier.string);
        auto maybe_field_value =
            field.decode(detail::make_itererator_reference(src), named_tuple{join(std::as_const(retval), ctx)}, ser);
        if (maybe_field_value) {
          retval[expr<field.identifier>] = *maybe_field_value;
        } else {
          err = maybe_field_value.error();
        }
      });
    }

    if (!err) {
      auto merged = named_tuple{join(std::as_const(retval), ctx)};
      m_fields.transform([&](const auto &field) { return field.deduce(retval, ser, m_fields); })
          .flatten()
          .for_each([&](const auto &named_value) {
            constexpr auto &id = named_value.identifier;
            if (!err && named_value.value() != merged[expr<id>]) {
              std::println("{} and {}", named_value, merged[expr<id>]);
              err = not_matching_deduction{id.string};
            }
          });
    }

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
  auto self_comparison_count = concatenated_fields.type_only()
                                   .transform([]<typename T>(typebox<T>) { return expr<T::identifier>; })
                                   .square()
                                   .transform(unpack | equal_to)
                                   .fold_left(expr<0uz>, plus);

  static_assert(self_comparison_count == concatenated_fields.size(),
                "Merging these descriptions would result in duplicate IDs");

  return std::move(concatenated_fields).apply([](auto &&...fields) { return description{std::move(fields)...}; });
}

} // namespace upd

namespace upd::descriptor {

constexpr inline auto empty_description = description<>{};

template<name Identifier, bool Signedness, std::size_t Width>
struct field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, xint<width>, xuint<width>>;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<named_tuple_like Packet, serializer Serializer, tuple_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
  }

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, const Packet &, Serializer &ser) -> result<value_type> {
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

template<bool Signedness, std::size_t Width>
struct unamed_field_t {
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using result_type = std::conditional_t<is_signed, xint<width>, xuint<width>>;

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr void encode(result_type value, Serializer &ser, OutputIt dest) const {
    if constexpr (is_signed) {
      ser.serialize_signed(value, dest);
    } else {
      ser.serialize_unsigned(value, dest);
    }
  }

  template<serializer Serializer, typename Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr auto decode(InputIt src, const Packet &, Serializer &ser) const -> result<result_type> {
    if constexpr (is_signed) {
      return ser.deserialize_signed(src, upd::width<width>);
    } else {
      return ser.deserialize_unsigned(src, upd::width<width>);
    }
  }
};

template<typename Enum, std::size_t Width>
struct unamed_enum_field_t {
  using enum_type = Enum;
  constexpr static auto width = Width;

  using result_type = xenum<enum_type, width>;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<enum_type>>;

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, const Packet &, Serializer &ser) -> result<result_type> {
    auto retval = [&] {
      if constexpr (is_signed) {
        return ser.deserialize_signed(src, upd::width<width - 1>);
      } else {
        return ser.deserialize_unsigned(src, upd::width<width>);
      }
    }();

    return result_type{retval};
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr static void encode(result_type value, Serializer &ser, OutputIt dest) {
    if constexpr (is_signed) {
      ser.serialize_signed(value.to_xint(), dest);
    } else {
      ser.serialize_unsigned(value.to_xint(), dest);
    }
  }
};

template<name Identifier, bool Signedness, std::size_t Width>
[[nodiscard]] constexpr auto field(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
  field_like auto retval = field_t<Identifier, Signedness, Width>{};

  return description{retval};
}

template<bool Signedness, std::size_t Width>
[[nodiscard]] constexpr auto field(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
  return unamed_field_t<Signedness, Width>{};
}

template<typename Enum, std::size_t Width>
[[nodiscard]] constexpr auto field(enumeration_t<Enum>, width_t<Width>) noexcept(release) {
  return unamed_enum_field_t<Enum, Width>{};
}

template<name Identifier, bool Signedness, std::size_t Width, typename Rule>
struct bound_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, xint<width>, xuint<width>>;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }
  using rule_type = Rule;

  rule_type rule;

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<named_tuple_like Packet, serializer Serializer, tuple_like Fields>
  [[nodiscard]] constexpr auto deduce(Packet &packet, Serializer &, const Fields &) const {
    return tagged_tuple{named_value{expr<identifier>, rule.deduce(std::as_const(packet))}};
  }

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, const Packet &, Serializer &ser) -> result<value_type> {
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

template<name Identifier, typename Enum, std::size_t Width, typename Rule>
struct enum_bound_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<Enum>>;
  constexpr static auto width = Width;

  using value_type = Enum;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }
  using rule_type = Rule;

  rule_type rule;

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<named_tuple_like Packet, serializer Serializer, tuple_like Fields>
  [[nodiscard]] constexpr auto deduce(Packet &packet, Serializer &, const Fields &) const {
    return tagged_tuple{named_value{expr<identifier>, rule.deduce(std::as_const(packet))}};
  }

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, const Packet &, Serializer &ser) -> result<value_type> {
    auto retval = [&] {
      if constexpr (is_signed) {
        return ser.deserialize_signed(src, upd::width<width - 1>);
      } else {
        return ser.deserialize_unsigned(src, upd::width<width>);
      }
    }();

    return static_cast<value_type>(retval);
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr static void encode(value_type value, Serializer &ser, OutputIt dest) {
    auto integral_value = std::to_underlying(value);
    auto xint_value = [&] {
      if constexpr (is_signed) {
        return xint{integral_value};
      } else {
        return xuint{integral_value};
      }
    }();

    if constexpr (is_signed) {
      ser.serialize_signed(xint_value, dest);
    } else {
      ser.serialize_unsigned(xint_value, dest);
    }
  }
};

template<name Identifier, bool Signedness, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto bound(signedness_t<Signedness>, width_t<Width>, Rule rule) noexcept(release) {
  auto retval = bound_t<Identifier, Signedness, Width, Rule>{std::move(rule)};

  return description{std::move(retval)};
}

template<name Identifier, typename Enum, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto bound(enumeration_t<Enum>, width_t<Width>, Rule rule) noexcept(release) {
  auto retval = enum_bound_t<Identifier, Enum, Width, Rule>{std::move(rule)};

  return description{std::move(retval)};
}

template<name Identifier, bool Signedness, std::size_t Width>
struct bound_elsewhere_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, xint<width>, xuint<width>>;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<named_tuple_like Packet, serializer Serializer, tuple_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
  }

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, const Packet &, Serializer &ser) -> result<value_type> {
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

template<name Identifier, typename Enum, std::size_t Width>
struct enum_bound_elsewhere_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<Enum>>;
  constexpr static auto width = Width;

  using value_type = Enum;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<named_tuple_like Packet, serializer Serializer, tuple_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
  }

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, const Packet &, Serializer &ser) -> result<value_type> {
    auto retval = [&] {
      if constexpr (is_signed) {
        return ser.deserialize_signed(src, upd::width<width>);
      } else {
        return ser.deserialize_unsigned(src, upd::width<width>);
      }
    }();

    return static_cast<value_type>(retval);
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr static void encode(value_type value, Serializer &ser, OutputIt dest) {
    auto integral_value = std::to_underlying(value);
    auto xint_value = [&] {
      if constexpr (is_signed) {
        return xint<width>{std::in_place, integral_value};
      } else {
        return xuint<width>{std::in_place, integral_value};
      }
    }();

    if constexpr (is_signed) {
      ser.serialize_signed(xint_value, dest);
    } else {
      ser.serialize_unsigned(xint_value, dest);
    }
  }
};

template<name Identifier, bool Signedness, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto bound(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
  auto retval = bound_elsewhere_t<Identifier, Signedness, Width>{};

  return description{retval};
}

template<name Identifier, typename Enum, std::size_t Width>
[[nodiscard]] constexpr auto bound(enumeration_t<Enum>, width_t<Width>) noexcept(release) {
  auto retval = enum_bound_elsewhere_t<Identifier, Enum, Width>{};

  return description{retval};
}

template<name Identifier, std::size_t Width>
struct constant_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = xuint<width>;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  value_type field_value;

  [[nodiscard]] constexpr auto default_value() const noexcept(release) -> value_type { return field_value; }

  template<named_tuple_like Packet, serializer Serializer, tuple_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
  }

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, const Packet &, Serializer &ser) -> result<value_type> {
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

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  BinaryOp op;
  value_type init;
  FieldFilter identifier_filter;

  [[nodiscard]] constexpr auto default_value() const noexcept(release) -> value_type { return init; }

  template<named_tuple_like Packet, serializer Serializer, tuple_like Fields>
  [[nodiscard]] constexpr auto deduce(Packet &packet, Serializer &ser, const Fields &fields) const {
    using namespace upd::literals;

    auto field_filter = [&](const auto &nv_and_field) {
      const auto &[nv, field] = nv_and_field;
      std::ignore = field;
      return UPD_INVOKE(identifier_filter, expr<nv.identifier>);
    };

    auto acc = make_value(0_x);
    auto f = [&](auto byte) { acc = UPD_INVOKE(op, acc, recompose_into_xuint(std::array{byte})); };
    auto it = invoker_iterator{&f};

    zip(packet, fields)
        .filter(field_filter)
        .transform([](const auto &nv_and_field) {
          const auto &[nv, field] = nv_and_field;
          return tuple{ref{nv.value()}, ref{field}};
        })
        .for_each([&](const auto &value_and_field) {
          const auto &[value, field] = value_and_field;
          field.encode(value, ser, it);
        });

    return tagged_tuple{named_value{expr<identifier>, acc}};
  }

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr static auto decode(InputIt src, const Packet &, Serializer &ser) -> result<value_type> {
    return ser.deserialize_unsigned(src, upd::width<width>);
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr static void encode(value_type value, Serializer &ser, OutputIt dest) {
    return ser.serialize_unsigned(value, dest);
  }
};

template<name Identifier, typename BinaryOp, std::size_t Width>
[[nodiscard]] constexpr auto checksum(BinaryOp op, xuint<Width> init, all_fields_t) noexcept(release) {
  auto is_not_this_field = [](auto id) { return expr<id != Identifier>; };

  auto retval =
      checksum_t<Identifier, BinaryOp, Width, decltype(is_not_this_field)>{std::move(op), init, is_not_this_field};

  return description{std::move(retval)};
}

template<auto Identifier, typename Rule, typename TaggedDescriptions>
struct one_of_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto size = std::tuple_size_v<TaggedDescriptions>;
  constexpr static auto alternative_types = equivalent_typelist_t<TaggedDescriptions>{}
                                                .metatransform([]<typename T>(T &&x) { return UPD_FWD(x).value(); })
                                                .metatransform([]<typename T>(T &&) -> typename T::result_type {})
                                                .chain_before(typebox<std::monostate>{})
                                                .to_typelist();

  using value_type = decltype(alternative_types.template metaapply<std::variant>());
  using tag_type = typename decltype(TaggedDescriptions::identifiers.apply([]<typename... Identifiers>(Identifiers...) {
    return typebox<std::common_type_t<typename Identifiers::value_type...>>{};
  }))::type;

  template<typename... Args>
    requires std::constructible_from<value_type, Args...>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  template<typename Choice>
    requires(is_instance_of<Choice, choice_t>())
  [[nodiscard]] constexpr auto make_value(Choice &&ch) const -> value_type {
    auto id_pos = tagged_descriptions.identifiers.find(expr<ch.code>);

    return UPD_FWD(ch).arguments.apply(
        [&](auto &&...args) { return value_type{std::in_place_index<id_pos + 1>, UPD_FWD(args)...}; });
  }

  using rule_type = Rule;

  rule_type rule;
  TaggedDescriptions tagged_descriptions;

  [[nodiscard]] constexpr static auto default_value() -> value_type { return value_type{}; }

  template<named_tuple_like Packet, serializer Serializer, tuple_like Fields>
  [[nodiscard]] constexpr auto deduce(const Packet &packet, Serializer &, const Fields &) const noexcept(release) {
    auto id_pos = packet[expr<identifier>].index() - 1;
    auto id = tagged_descriptions.identifiers.visit(id_pos, [&](auto id) -> tag_type { return id; });
    return tagged_tuple{keyword<rule.from_identifier>{} = UPD_INVOKE(inverse(rule.chain), id)};
  }

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr auto decode(InputIt src, const Packet &packet, Serializer &ser) const -> result<value_type> {
    namespace stdr = std::ranges;

    auto id = rule.deduce(packet);
    auto id_pos = tagged_descriptions.identifiers.find(id);

    if (id_pos == tagged_descriptions.size()) {
      return std::unexpected{invalid_code_in_one_of{identifier.string, std::to_underlying(id)}};
    }

    auto make_alt = [&](const auto &id_and_descr) {
      const auto &[id, descr] = id_and_descr;
      const auto id_pos = tagged_descriptions.identifiers.find(id);
      auto make_retval = [&](auto &&alt) { return value_type{std::in_place_index<id_pos + 1>, UPD_FWD(alt)}; };
      auto retval = descr.decode(src, packet, ser).transform(make_retval);
      return retval;
    };

    return tagged_descriptions.visit(id_pos, make_alt);
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr void encode(const value_type &value, Serializer &ser, OutputIt dest) const noexcept(release) {
    auto alt_index = value.index();
    auto encode_alt = [&](const auto &i_and_named_descr) {
      const auto &[i, named_descr] = i_and_named_descr;
      const auto *alt = std::get_if<i.value + 1>(&value);

      UPD_ASSERT(alt);

      named_descr.value().encode(*alt, ser, dest);
    };

    return zip(sequence<size>, tagged_descriptions).visit(alt_index - 1, encode_alt);
  }
};

template<name Identifier, typename Rule, typename... WhenThens>
[[nodiscard]] constexpr auto one_of(Rule &&rule, WhenThens &&...when_thens) {
  using rule_type = std::remove_cvref_t<Rule>;

  auto tagged_descriptions = aggregate_when_thens(UPD_FWD(when_thens)...);
  auto retval = one_of_t<Identifier, rule_type, decltype(tagged_descriptions)>{
      .rule = UPD_FWD(rule),
      .tagged_descriptions = std::move(tagged_descriptions),
  };

  return description{std::move(retval)};
}

template<name Identifier, typename Description, typename Rule, std::size_t Max>
struct repeat_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto max = Max;

  using value_type = static_vector<typename Description::result_type, Max>;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }
  using description_type = Description;
  using rule_type = Rule;

  Description description;
  Rule rule;

  [[nodiscard]] constexpr static auto default_value() -> value_type { return value_type{}; }

  template<named_tuple_like Packet, serializer Serializer, tuple_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
  }

  template<serializer Serializer, named_tuple_like Packet, std::input_iterator InputIt>
  [[nodiscard]] constexpr auto decode(InputIt src, const Packet &packet, Serializer &ser) const -> result<value_type> {
    namespace stdv = std::views;

    auto count = rule.deduce(packet);
    if (count < 0) {
      return std::unexpected{negative_repetition_count{identifier.string, try_cast<std::intmax_t>(count).value_or(0)}};
    }
    if (Max < count) {
      return std::unexpected{repeated_beyond_max{identifier.string, static_cast<std::uintmax_t>(count), Max}};
    }

    auto retval = static_vector<typename Description::result_type, Max>{};

    for (auto _ : stdv::iota(0uz, std::size_t(count))) {
      auto maybe_value = description.decode(src, packet, ser);
      if (!maybe_value) {
        return std::unexpected{std::move(maybe_value).error()};
      }
      retval.push_back(std::move(maybe_value).value());
    }

    return retval;
  }

  template<serializer Serializer, std::output_iterator<byte_type<Serializer>> OutputIt>
  constexpr void encode(const value_type &value, Serializer &ser, OutputIt dest) const {
    for (const auto &element : value) {
      description.encode(element, ser, dest);
    }
  }
};

template<name Identifier, typename Description, typename Rule, std::size_t Max>
[[nodiscard]] constexpr auto repeat(Description &&descr, Rule &&rule, at_most_t<Max>) {
  using description_type = std::remove_cvref_t<Description>;
  using rule_type = std::remove_cvref_t<Rule>;

  auto retval = repeat_t<Identifier, description_type, rule_type, Max>{
      .description = UPD_FWD(descr),
      .rule = UPD_FWD(rule),
  };

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
