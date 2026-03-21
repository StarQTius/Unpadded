#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "algebra/side.hpp"
#include "algebra/variable.hpp"
#include "concept/invocable.hpp"
#include "constexpr.hpp"
#include "description/serializer.hpp"
#include "error.hpp"
#include "get.hpp"
#include "record/concat.hpp"
#include "record/entry.hpp"
#include "record/find.hpp"
#include "record/fold.hpp"
#include "record/for_each.hpp"
#include "record/get_ith.hpp"
#include "record/has_tag.hpp"
#include "record/instantiate.hpp"
#include "record/join.hpp"
#include "record/name.hpp"
#include "record/record.hpp"
#include "record/record_like.hpp"
#include "record/take.hpp"
#include "record/to.hpp"
#include "record/transform.hpp"
#include "record/universal_record.hpp"
#include "record/values.hpp"
#include "safe_operation.hpp"
#include "static_vector.hpp"
#include "stream_interface.hpp"
#include "template_traits.hpp"
#include "token.hpp"
#include "tuple/as_record.hpp"
#include "tuple/concat.hpp"
#include "tuple/join.hpp"
#include "tuple/transform.hpp"
#include "tuple/tuple_like.hpp"
#include "tuple/tuple_size.hpp"
#include "tuple/tuple_view_adaptor.hpp"
#include "tuple/typelist.hpp"
#include "tuple/visit.hpp"
#include "upd.hpp"
#include "with_sequence.hpp"

namespace upd::descriptor {

template<name, bool, std::size_t>
struct field_t; // IWYU pragma: keep

template<name, bool, std::size_t, typename>
struct bound_t; // IWYU pragma: keep

template<bool, std::size_t>
struct anonymous_field_t; // IWYU pragma: keep

template<name, std::size_t>
struct constant_t; // IWYU pragma: keep

template<name, typename, std::size_t, typename>
struct checksum_t; // IWYU pragma: keep

} // namespace upd::descriptor

namespace upd {

struct defval_t {
  template<std::default_initializable T>
  [[nodiscard]] constexpr operator T() const noexcept(release) {
    return T{};
  }
};

constexpr auto defval = defval_t{};

template<auto Tag, record_like Record, typename T>
[[nodiscard]] constexpr auto get_or(Record &&rec, T &&x) -> decltype(auto) {
  if constexpr (has_tag<Tag>(rec)) {
    return get<Tag>(UPD_FWD(rec));
  } else {
    return UPD_FWD(x);
  }
}

template<auto Code, typename... Args>
struct choice_t {
  constexpr static auto code = Code;
  constexpr static auto argument_types = typelist2<Args...>;

  std::tuple<Args...> arguments;
};

template<auto Code, typename... Args>
[[nodiscard]] constexpr auto choice(Args &&...args) -> choice_t<Code, Args...> {
  return choice_t<Code, Args...>{.arguments = {UPD_FWD(args)...}};
}

template<name Identifier, std::size_t Width>
[[nodiscard]] constexpr auto bitsize(std::uintmax_t, descriptor::field_t<Identifier, false, Width>) noexcept(release)
    -> std::size_t {
  return Width;
}

template<name Identifier, std::size_t Width>
[[nodiscard]] constexpr auto bitsize(std::uintmax_t, descriptor::constant_t<Identifier, Width>) noexcept(release)
    -> std::size_t {
  return Width;
}

template<name Identifier, std::size_t Width>
[[nodiscard]] constexpr auto bitsize(std::intmax_t, descriptor::field_t<Identifier, true, Width>) noexcept(release)
    -> std::size_t {
  return Width;
}

template<name Identifier, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto bitsize(std::uintmax_t,
                                     descriptor::bound_t<Identifier, false, Width, Rule>) noexcept(release)
    -> std::size_t {
  return Width;
}

template<name Identifier, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto bitsize(std::intmax_t,
                                     descriptor::bound_t<Identifier, true, Width, Rule>) noexcept(release)
    -> std::size_t {
  return Width;
}

template<std::size_t Width>
[[nodiscard]] constexpr auto bitsize(std::uintmax_t, descriptor::anonymous_field_t<false, Width>) noexcept(release)
    -> std::size_t {
  return Width;
}

template<std::size_t Width>
[[nodiscard]] constexpr auto bitsize(std::intmax_t, descriptor::anonymous_field_t<true, Width>) noexcept(release)
    -> std::size_t {
  return Width;
}

template<name Identifier, typename BinaryOp, std::size_t Width, typename FieldFilter>
[[nodiscard]] constexpr auto bitsize(std::uintmax_t,
                                     descriptor::checksum_t<Identifier, BinaryOp, Width, FieldFilter>) noexcept(release)
    -> std::size_t {
  return Width;
}

template<typename Enum, typename Field>
  requires std::is_enum_v<Enum>
[[nodiscard]] constexpr auto bitsize(Enum, const Field &field) noexcept(release) -> std::size_t {
  return field.width;
}

template<typename... Ts, typename Field>
[[nodiscard]] constexpr auto bitsize(const std::variant<Ts...> &sum_of_field_values,
                                     const Field &field) noexcept(release) -> std::size_t {
  namespace updv = upd::tuple_views;
  auto alt_index = sum_of_field_values.index();
  if (alt_index == 0) {
    return 0;
  }

  auto seq = UPD_WITH_SEQUENCE(Is, sizeof...(Ts) - 1, &) { return std::tuple{expr<Is>...}; };
  return updv::visit(seq, alt_index - 1, [&](auto i) {
    const auto &field_value = *std::get_if<i + 1>(&sum_of_field_values);
    const auto &alt_descr = get_ith<i>(field.tagged_descriptions);
    return bitsize(field_value, alt_descr);
  });
}

template<record_like NamedFieldValues, typename Description>
[[nodiscard]] constexpr auto bitsize(const NamedFieldValues &named_field_values,
                                     const Description &descr) noexcept(release) -> std::size_t {
  namespace updv = upd::record_views;
  return updv::fold_left(named_field_values, 0uz, [&](std::size_t acc, auto k, const auto &field_value) {
    auto field_pos = updv::find_if(descr.m_fields, [&](auto id, const auto &) { return expr<id == k>; });
    return acc + bitsize(field_value.value(), descr.m_fields[field_pos]);
  });
}

template<typename... Entries, typename Description>
[[nodiscard]] constexpr auto bitsize(const record<Entries...> &named_field_values,
                                     const Description &descr) noexcept(release) -> std::size_t {
  namespace updv = record_views;
  return updv::fold_left(named_field_values, 0uz, [&](std::size_t acc, auto k, const auto &field_value) {
    auto field_pos = updv::find_if(descr.m_fields, [&](auto id, auto) { return expr<id == k>; });
    return acc + bitsize(field_value, get_ith<field_pos>(descr.m_fields));
  });
}

template<typename T, std::size_t Max, typename Field>
[[nodiscard]] constexpr static auto bitsize(const static_vector<T, Max> &svec, const Field &field) -> std::size_t {
  namespace stdr = std::ranges;
  return stdr::fold_left(svec, 0uz, [&](auto acc, const auto &elem) { return acc + bitsize(elem, field.description); });
}

template<typename T, std::size_t N, typename Field>
[[nodiscard]] constexpr static auto bitsize(const std::array<T, N> &arr, const Field &field) -> std::size_t {
  namespace stdr = std::ranges;
  return stdr::fold_left(arr, 0uz, [&](auto acc, const auto &elem) { return acc + bitsize(elem, field.description); });
}

enum class vartype {
  value,
  length,
  count,
  code,
};

template<std::size_t N>
struct varname {
  vartype type;
  name<N> id;
};

template<name Identifier>
constexpr auto value_of = upd::algebra::side{upd::algebra::variable<varname{vartype::value, Identifier}>{}};

template<name Identifier>
constexpr auto length_of = upd::algebra::side{upd::algebra::variable<varname{vartype::length, Identifier}>{}};

template<name Identifier>
constexpr auto count_of = upd::algebra::side{upd::algebra::variable<varname{vartype::count, Identifier}>{}};

template<name Identifier>
constexpr auto code_of = upd::algebra::side{upd::algebra::variable<varname{vartype::code, Identifier}>{}};

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
  return record{entry<when_thens.match, typename WhenThens::result_type>{UPD_FWD(when_thens).result}...};
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

template<typename T>
concept field_like = true;

template<typename T, typename Serializer>
concept deducible_field = field_like<T> && serializer<Serializer> && requires(T x, Serializer ser, record<> packet) {
  { x.deduce(packet, ser) } -> std::same_as<void>;
};

template<typename T, typename Serializer>
concept decodable_field = serializer<Serializer> && deducible_field<T, Serializer> &&
                          requires(T x, Serializer ser, const record<> packet, const byte_type<Serializer> *src) {
                            { x.decode(src, ser, packet) } -> std::same_as<typename T::value_type>;
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
  using result_type =
      decltype(typelist2<Ts...> |
               tuple_views::transform_type([]<typename T> -> entry<T::identifier, typename T::value_type> {}) |
               tuple_views::as_record | record_views::instantiate<record>);

  using storage_type =
      decltype(typelist2<Ts...> | tuple_views::transform_type([]<typename T> -> entry<T::identifier, T> {}) |
               tuple_views::as_record | record_views::instantiate<record>);

  using input_type =
      decltype(typelist2<Ts...> |
               tuple_views::transform_type([]<typename T> -> entry<T::identifier, typename T::input_type> {}) |
               tuple_views::as_record | record_views::instantiate<record>);

  explicit constexpr description(Ts... fields) : m_fields{entry{expr<fields.identifier>, std::move(fields)}...} {}

  template<record_like Args, serializer Serializer>
  constexpr void encode(const Args &args, Serializer &ser, std::ostream &dest, const char *sep) const {
    encode(args, ser, standard_stream{nullptr, &dest, sep});
  }

  template<record_like Args, serializer Serializer, tuple_like2 System = std::tuple<>>
  constexpr void
  encode(const Args &args, Serializer &ser, stream_interface &dest, const System &ctx_sys = std::tuple{}) const {
    namespace updv = record_views;

    auto rule_sys = m_fields | updv::values |
                    tuple_views::transform([&](const auto &field) { return field.rules(args, m_fields, ser); }) |
                    tuple_views::join;

    auto sys = tuple_views::concat(rule_sys, ctx_sys);

    updv::for_each(m_fields, [&](auto id, const auto &field) {
      ser.checkpoint(id.value.string);
      field.encode(get_or<id.value>(args, defval), ser, dest, sys);
    });
  }

  template<record_like Args, serializer Serializer>
  constexpr void encode(const Args &args, Serializer &ser, stream_interface &&dest) const {
    encode(args, ser, dest);
  }

  template<std::input_iterator InputIt,
           serializer Serializer,
           record_like Context = upd::record<>,
           tuple_like2 System = std::tuple<>>
    requires std::convertible_to<std::iter_value_t<InputIt>, word_t>
  [[nodiscard]] constexpr auto
  decode(InputIt src, Serializer &ser, const Context &ctx = record{}, const System &sys = std::tuple{}) const {
    auto null_it = static_cast<word_t *>(nullptr);
    return decode(iterator_stream{src, null_it}, ser, ctx, sys);
  }

  template<std::input_iterator InputIt,
           serializer Serializer,
           record_like Context = upd::record<>,
           tuple_like2 System = std::tuple<>>
    requires std::same_as<std::iter_value_t<InputIt>, std::byte>
  [[nodiscard]] constexpr auto
  decode(InputIt src, Serializer &ser, const Context &ctx = record{}, const System &sys = std::tuple{}) const {
    namespace stdr = std::ranges;
    namespace stdv = std::views;

    auto words = stdr::subrange(src, std::unreachable_sentinel) | stdv::transform(std::to_integer<word_t>);

    auto null_it = static_cast<word_t *>(nullptr);
    return decode(iterator_stream{std::begin(words), null_it}, ser, ctx, sys);
  }

  template<serializer Serializer, record_like Context = upd::record<>, tuple_like2 System = std::tuple<>>
  [[nodiscard]] constexpr auto decode(stream_interface &src,
                                      Serializer &ser,
                                      const Context &ctx = record{},
                                      const System &presys = std::tuple{}) const {
    namespace updv = record_views;

    auto err = error{};
    auto retval =
        m_fields | updv::transform([&](auto, const auto &field) { return field.default_value(); }) | updv::to<record>;

    updv::for_each(ctx, [&](auto id, const auto &value) { get<id.value>(retval) = value; });

    if (!err) {
      updv::for_each(m_fields, [&](auto id, const auto &field) {
        ser.checkpoint(id.value.string);
        auto ctx_ = [&] {
          if constexpr (has_tag<id.value>(ctx)) {
            return updv::concat(retval | updv::take_until<id.value>, entry{id, get<id.value>(ctx)}) |
                   updv::to<upd::record>;
          } else {
            return retval | updv::take_until<id.value>;
          }
        }();
        auto sys = tuple_views::concat(
            presys,
            get<id.value>(m_fields).rules(ctx_, m_fields, ser),
            m_fields | updv::take_until<id.value> | updv::values | tuple_views::transform([&](const auto &field) {
              return field.rules(ctx_, m_fields, ser);
            }) | tuple_views::join);
        auto maybe_field_value = field.decode(src, ser, retval, m_fields, sys);
        if (maybe_field_value) {
          get<id.value>(retval) = *maybe_field_value;
        } else {
          err = maybe_field_value.error();
        }
      });
    }

    auto sys = tuple_views::concat(presys, m_fields | updv::values | tuple_views::transform([&](const auto &field) {
                                             return field.rules(retval, m_fields, ser);
                                           }) | tuple_views::join);

    if (!err) {
      auto merged = updv::concat(std::as_const(retval), ctx);
      auto deduced =
          m_fields |
          updv::transform([&](auto, const auto &field) { return field.deduce(retval, ser, m_fields, sys); }) |
          updv::join([](auto, auto k) { return k; }) | updv::to<record>;

      updv::for_each(deduced, [&](auto id, const auto &ded) {
        const auto &actual = get<id.value>(merged);
        if (!err && safe_not_equal(actual, ded)) {
          err = not_matching_deduction{
              id.value.string, static_cast<std::intmax_t>(actual), static_cast<std::intmax_t>(ded)};
        }
      });
    }

    return result_if_no_error(std::move(retval), std::move(err));
  }

  template<serializer Serializer, record_like Context = upd::record<>, tuple_like2 System = std::tuple<>>
  [[nodiscard]] constexpr auto decode(stream_interface &&src,
                                      Serializer &ser,
                                      const Context &ctx = record{},
                                      const System &sys = std::tuple{}) const {
    return decode(src, ser, ctx, sys);
  }

  [[nodiscard]] constexpr auto length() const noexcept(release) {
    namespace updv = upd::record_views;
    return updv::fold_left(m_fields | updv::transform([](auto, const auto &field) { return field.length(); }),
                           0uz,
                           [](auto acc, auto, auto len) { return acc + len; });
  }

  storage_type m_fields;
};

template<typename... Ts, typename... Us>
[[nodiscard]] constexpr auto operator|(description<Ts...> lhs, description<Us...> rhs) noexcept(release) {
  namespace updv = record_views;

  auto fields = updv::concat(std::move(lhs.m_fields), std::move(rhs.m_fields)) | updv::values;
  return UPD_WITH_SEQUENCE(Is, tuple_size_v<decltype(fields)>, &) {
    return description{get<Is>(std::move(fields))...};
  };
}

} // namespace upd

namespace upd::descriptor {

constexpr inline auto empty_description = description<>{};

template<bool Signedness, std::size_t Width>
struct unamed_field_t {
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using result_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;

  template<serializer Serializer>
  constexpr void encode(result_type value, Serializer &ser, stream_interface &dest) const {
    if constexpr (is_signed) {
      ser.serialize_signed(value, upd::width<width>, dest);
    } else {
      ser.serialize_unsigned(value, upd::width<width>, dest);
    }
  }

  template<serializer Serializer, typename Packet>
  [[nodiscard]] constexpr auto decode(stream_interface &src, Serializer &ser, const Packet &) const
      -> result<result_type> {
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

  using result_type = enum_type;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<enum_type>>;

  template<serializer Serializer, record_like Packet>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Packet &)
      -> result<result_type> {
    auto retval = [&] {
      if constexpr (is_signed) {
        return ser.deserialize_signed(src, upd::width<width - 1>);
      } else {
        return ser.deserialize_unsigned(src, upd::width<width>);
      }
    }();

    return result_type{retval};
  }

  template<serializer Serializer>
  constexpr static void encode(result_type value, Serializer &ser, stream_interface &dest) {
    auto underlying_value = std::to_underlying(value);

    if constexpr (is_signed) {
      ser.serialize_signed(underlying_value, upd::width<width - 1>, dest);
    } else {
      ser.serialize_unsigned(underlying_value, upd::width<width>, dest);
    }
  }
};

template<bool Signedness, std::size_t Width>
[[nodiscard]] constexpr auto field(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
  return unamed_field_t<Signedness, Width>{};
}

template<typename Enum, std::size_t Width>
[[nodiscard]] constexpr auto field(enumeration_t<Enum>, width_t<Width>) noexcept(release) {
  return unamed_enum_field_t<Enum, Width>{};
}

template<name Identifier, typename Enum, std::size_t Width, typename Rule>
struct enum_bound_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<Enum>>;
  constexpr static auto width = Width;

  using value_type = Enum;

  template<record_like Context, typename... Args>
  [[nodiscard]] constexpr auto make_value(const Context &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }
  using rule_type = Rule;

  rule_type rule;

  template<record_like Context>
  [[nodiscard]] constexpr static auto default_value(const Context &) noexcept(release) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr auto deduce(Packet &packet, Serializer &, const Fields &fields) const {
    return record{entry{expr<identifier>, rule.deduce(std::as_const(packet), fields)}};
  }

  template<serializer Serializer, record_like Packet, record_like Fields>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Packet &, const Fields &)
      -> result<value_type> {
    auto retval = [&] {
      if constexpr (is_signed) {
        return ser.deserialize_signed(src, upd::width<width - 1>);
      } else {
        return ser.deserialize_unsigned(src, upd::width<width>);
      }
    }();

    return static_cast<value_type>(retval);
  }

  template<serializer Serializer>
  constexpr static void encode(value_type value, Serializer &ser, stream_interface &dest) {
    auto integral_value = std::to_underlying(value);
    if constexpr (is_signed) {
      ser.serialize_signed(integral_value, upd::width<width>, dest);
    } else {
      ser.serialize_unsigned(integral_value, upd::width<width>, dest);
    }
  }
};

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

  using value_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;

  template<record_like Context, typename... Args>
  [[nodiscard]] constexpr auto make_value(const Context &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  template<record_like Context>
  [[nodiscard]] constexpr static auto default_value(const Context &) noexcept(release) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return record{};
  }

  template<serializer Serializer, record_like Packet, record_like Fields>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Packet &, const Fields &)
      -> result<value_type> {
    if constexpr (is_signed) {
      return ser.deserialize_signed(src, upd::width<width>);
    } else {
      return ser.deserialize_unsigned(src, upd::width<width>);
    }
  }

  template<serializer Serializer>
  constexpr static void encode(value_type value, Serializer &ser, stream_interface &dest) {
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

  template<record_like Context, typename... Args>
  [[nodiscard]] constexpr auto make_value(const Context &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  template<record_like Context>
  [[nodiscard]] constexpr static auto default_value(const Context &) noexcept(release) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return record{};
  }

  template<serializer Serializer, record_like Packet, record_like Fields>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Packet &, const Fields &)
      -> result<value_type> {
    auto retval = [&] {
      if constexpr (is_signed) {
        return ser.deserialize_signed(src, upd::width<width>);
      } else {
        return ser.deserialize_unsigned(src, upd::width<width>);
      }
    }();

    return static_cast<value_type>(retval);
  }

  template<serializer Serializer>
  constexpr static void encode(value_type value, Serializer &ser, stream_interface &dest) {
    auto integral_value = std::to_underlying(value);
    if constexpr (is_signed) {
      ser.serialize_signed(integral_value, upd::width<width>, dest);
    } else {
      ser.serialize_unsigned(integral_value, upd::width<width>, dest);
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

template<>
struct std::formatter<upd::defval_t> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  constexpr static auto format(upd::defval_t, std::format_context &ctx) {
    auto it = ctx.out();

    it = std::format_to(it, "defval");

    ctx.advance_to(it);
    return it;
  }
};

template<std::size_t N>
struct std::formatter<upd::varname<N>> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(upd::varname<N> vn, std::format_context &ctx) {
    auto it = ctx.out();

    switch (vn.type) {
    case upd::vartype::value:
      it = std::format_to(it, "[value of ");
      break;
    case upd::vartype::length:
      it = std::format_to(it, "[length of ");
      break;
    case upd::vartype::count:
      it = std::format_to(it, "[count of ");
      break;
    case upd::vartype::code:
      it = std::format_to(it, "[code of ");
      break;
    default:
      it = std::format_to(it, "[??? of ");
      break;
    };
    it = std::format_to(it, "{}]", vn.id);

    ctx.advance_to(it);
    return it;
  }
};
