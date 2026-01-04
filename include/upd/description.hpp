#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
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
#include "is_instance_of.hpp"
#include "record/concat.hpp"
#include "record/entry.hpp"
#include "record/filter.hpp"
#include "record/find.hpp"
#include "record/fold.hpp"
#include "record/for_each.hpp"
#include "record/get_ith.hpp"
#include "record/has_tag.hpp"
#include "record/instantiate.hpp"
#include "record/join.hpp"
#include "record/name.hpp"
#include "record/record.hpp"
#include "record/record_element.hpp"
#include "record/record_like.hpp"
#include "record/record_size.hpp"
#include "record/tags.hpp"
#include "record/tags_of.hpp"
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
#include "tuple/apply.hpp"
#include "tuple/as_record.hpp"
#include "tuple/concat.hpp"
#include "tuple/enumerate.hpp"
#include "tuple/find.hpp"
#include "tuple/fold.hpp"
#include "tuple/join.hpp"
#include "tuple/reverse.hpp"
#include "tuple/to.hpp"
#include "tuple/transform.hpp"
#include "tuple/tuple_like.hpp"
#include "tuple/tuple_size.hpp"
#include "tuple/tuple_view_adaptor.hpp"
#include "tuple/typelist.hpp"
#include "tuple/visit.hpp"
#include "type_traits.hpp"
#include "upd.hpp"
#include "with_sequence.hpp"

namespace upd {

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
  std::tuple<Inversibles...> operations;

  template<typename Self, typename U>
  [[nodiscard]] constexpr auto operator()(this Self &&self, U x) {
    namespace updv = upd::tuple_views;

    return updv::fold_left(UPD_FWD(self).operations, x, [](auto acc, auto &&op) { return UPD_INVOKE(op, acc); });
  };

  template<typename Self, inversible Inversible>
  [[nodiscard]] constexpr auto and_then(this Self &&self, Inversible &&op) {
    namespace updv = upd::tuple_views;

    return updv::apply(UPD_FWD(self).operations, [&](auto &&...ops) {
      return bijective_chain<Inversibles..., Inversible>{{UPD_FWD(ops)..., UPD_FWD(op)}};
    });
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
  namespace updv = upd::tuple_views;

  auto inv_ops = chain.operations | updv::transform([](const auto &op) { return inverse(op); }) | updv::reverse |
                 updv::to<std::tuple>;

  return bijective_chain{std::move(inv_ops)};
}

template<inversible... Inversibles>
[[nodiscard]] constexpr auto inverse(bijective_chain<Inversibles...> &&chain) {
  namespace updv = upd::tuple_views;

  auto inv_ops = std::move(chain).operations | updv::transform([](auto &&op) { return inverse(std::move(op)); }) |
                 updv::reverse | updv::to<std::tuple>;

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

  template<typename Self, typename T, typename Fields>
  [[nodiscard]] constexpr auto deduce(this Self &&self, T &&from, const Fields &fields) {
    decltype(auto) value = UPD_INVOKE(UPD_FWD(self).get_value, UPD_FWD(from), fields);
    return UPD_INVOKE(UPD_FWD(self).chain, UPD_FWD(value));
  }

  template<typename Self, inversible Inversible>
  [[nodiscard]] constexpr auto and_then(this Self &&self, Inversible &&op) {
    return field_expression_t<Identifier, F, Inversibles..., Inversible>{
        self.id_const, UPD_FWD(self).get_value, UPD_FWD(self).chain.and_then(UPD_FWD(op))};
  }
};

template<typename Field>
[[nodiscard]] constexpr auto bitsize(std::uintmax_t, const Field &field) noexcept(release) -> std::size_t {
  return field.width;
}

template<typename Field>
[[nodiscard]] constexpr auto bitsize(std::intmax_t, const Field &field) noexcept(release) -> std::size_t {
  return field.width;
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

  explicit constexpr description(Ts... fields) : m_fields{entry{expr<fields.identifier>, std::move(fields)}...} {}

  template<record_like Args, serializer Serializer>
  constexpr void encode(const Args &args, Serializer &ser, std::ostream &dest, const char *sep) const {
    encode(args, ser, standard_stream{nullptr, &dest, sep});
  }

  template<record_like Args, serializer Serializer>
  constexpr void encode(const Args &args, Serializer &ser, stream_interface &dest) const {
    namespace updv = record_views;

    auto presys = m_fields | updv::values |
                  tuple_views::transform([&](const auto &field) { return field.rules(args, m_fields); }) |
                  tuple_views::join | tuple_views::to<std::tuple>;

    auto presys2 = args | updv::filter([]<auto Tag, typename T>(expr_t<Tag>, typebox<T>) {
                     return std::is_scalar_v<std::remove_cvref_t<T>>;
                   }) |
                   updv::transform([](auto id, auto v) { return value_of<id.value> = v; }) | updv::values |
                   tuple_views::to<std::tuple>;

    auto packet = m_fields | updv::transform([&]<typename Field>(auto, const Field &field) {
                    if constexpr (has_tag<field.identifier>(args)) {
                      return field.make_value(tuple_views::concat(presys, presys2), get<field.identifier>(args));
                    } else {
                      return field.default_value(tuple_views::concat(presys, presys2));
                    }
                  }) |
                  updv::to<record>;

    auto sys = m_fields | updv::values |
               tuple_views::transform([&](const auto &field) { return field.rules(packet, m_fields); }) |
               tuple_views::join | tuple_views::to<std::tuple>;

    updv::for_each(m_fields, [&](auto id, const auto &field) {
      auto postsys = packet | updv::take_until<id.value> |
                     updv::filter([]<auto Tag, typename T>(expr_t<Tag>, typebox<T>) {
                       return std::is_scalar_v<std::remove_cvref_t<T>>;
                     }) |
                     updv::transform([](auto id, auto v) { return value_of<id.value> = v; }) | updv::values |
                     tuple_views::to<std::tuple>;
      updv::for_each(field.deduce(packet, ser, m_fields, tuple_views::concat(sys, postsys)),
                     [&](auto id, const auto &named_value) { packet[keyword2<id.value>{}] = named_value; });
    });

    updv::for_each(m_fields, [&](auto id, const auto &field) {
      ser.checkpoint(id.value.string);
      field.encode(packet[keyword2<id.value>{}], ser, dest);
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

    auto sys = tuple_views::concat(presys, m_fields | updv::values | tuple_views::transform([&](const auto &field) {
                                             return field.rules(retval, m_fields);
                                           }) | tuple_views::join) |
               tuple_views::to<std::tuple>;

    if (!err) {
      updv::for_each(m_fields, [&](auto id, const auto &field) {
        ser.checkpoint(id.value.string);
        auto packet = updv::concat(std::as_const(retval), ctx);
        auto postsys = retval | updv::take_until<id.value> |
                       updv::filter([]<auto Tag, typename T>(expr_t<Tag>, typebox<T>) {
                         return std::is_scalar_v<std::remove_cvref_t<T>>;
                       }) |
                       updv::transform([](auto id, auto v) { return value_of<id.value> = v; }) | updv::values |
                       tuple_views::to<std::tuple>;
        auto maybe_field_value = field.decode(src, ser, packet, m_fields, tuple_views::concat(sys, postsys));
        if (maybe_field_value) {
          get<id.value>(retval) = *maybe_field_value;
        } else {
          err = maybe_field_value.error();
        }
      });
    }

    if (!err) {
      auto merged = updv::concat(std::as_const(retval), ctx);
      auto postsys = retval | updv::filter([]<auto Tag, typename T>(expr_t<Tag>, typebox<T>) {
                       return std::is_scalar_v<std::remove_cvref_t<T>>;
                     }) |
                     updv::transform([](auto id, auto v) { return value_of<id.value> = v; }) | updv::values |
                     tuple_views::to<std::tuple>;
      auto deduced = m_fields | updv::transform([&](auto, const auto &field) {
                       return field.deduce(retval, ser, m_fields, tuple_views::concat(sys, postsys));
                     }) |
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

template<auto Identifier, typename Rule, typename TaggedDescriptions>
struct one_of_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto size = record_size_v<TaggedDescriptions>;
  constexpr static auto alternative_types =
      decltype(tuple_views::concat(typelist2<upd::description<>>,
                                   std::declval<TaggedDescriptions>() | record_views::values |
                                       tuple_views::to<typelist2_t>) |
               tuple_views::transform_type([]<typename T> -> std::remove_cvref_t<T> {}) |
               tuple_views::transform_type([]<typename T> -> typename T::result_type {}) |
               tuple_views::to<typelist2_t>){};

  using value_type = decltype(tuple_views::apply_type(alternative_types, []<typename... Ts> -> std::variant<Ts...> {}));

  using tag_type = decltype(tuple_views::apply_type(
      tags_of_v<TaggedDescriptions> | tuple_views::transform_type([]<typename T> -> typename T::value_type {}),
      []<typename... Ts> -> std::common_type_t<Ts...> {}));

  template<tuple_like2 System, typename... Args>
  [[nodiscard]] constexpr auto make_value(const System &sys, Args &&...args) const -> value_type {
    namespace updv = upd::tuple_views;

    auto seq = UPD_WITH_SEQUENCE(Is, size) { return std::tuple{expr<Is>...}; };

    auto code = solve_for(code_of<Identifier>, sys);
    auto i = updv::dynfind(tags_of_v<TaggedDescriptions>, code);

    return updv::visit(seq, i, [&](auto i) -> value_type {
      if constexpr (std::constructible_from<value_type, std::in_place_index_t<i + 1>, Args...>) {
        return value_type{std::in_place_index<i + 1>, UPD_FWD(args)...};
      } else {
        UPD_ASSERT(false);
      }
    });
  }

  using rule_type = Rule;

  rule_type rule;
  TaggedDescriptions tagged_descriptions;

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto default_value(const System &sys) const -> value_type {
    return make_value(sys);
  }

  [[nodiscard]] constexpr auto default_value() const -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr auto deduce(const Packet &, Serializer &, const Fields &, const System &) const
      noexcept(release) {
    return record{};
  }

  template<record_like Packet, record_like Fields>
  [[nodiscard]] constexpr auto rules(const Packet &packet, const Fields &fields) const {
    return std::tuple{code_of<Identifier> = rule, length_of<Identifier> = [&] {
                        if constexpr (has_tag<Identifier>(packet)) {
                          if constexpr (is_instance_of<record_element_t<Identifier, Packet>, std::variant>()) {
                            return bitsize(get<Identifier>(packet), get<Identifier>(fields));
                          } else {
                            return 0uz;
                          }
                        } else {
                          return 0uz;
                        }
                      }};
  }

  template<serializer Serializer, record_like Packet, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, Serializer &ser, const Packet &packet, const Fields &, const System &sys) const
      -> result<value_type> {
    namespace stdr = std::ranges;
    namespace updv = upd::tuple_views;

    auto id = solve_for(code_of<Identifier>, sys);
    auto id_pos = updv::dynfind(tagged_descriptions | record_views::tags, id);

    if (id_pos == record_size_v<TaggedDescriptions>) {
      return std::unexpected{invalid_code_in_one_of{identifier.string, std::to_underlying(id)}};
    }

    auto make_alt = [&](const auto &id_pos_and_descr) {
      const auto &[id_pos, descr] = id_pos_and_descr;
      auto make_retval = [&](auto &&alt) { return value_type{std::in_place_index<id_pos + 1>, UPD_FWD(alt)}; };
      auto retval = descr.decode(src, ser, packet, sys).transform(make_retval);
      return retval;
    };

    return updv::visit(tagged_descriptions | record_views::values | updv::enumerate, id_pos, make_alt);
  }

  template<serializer Serializer>
  constexpr void encode(const value_type &value, Serializer &ser, stream_interface &dest) const noexcept(release) {
    namespace updv = upd::tuple_views;

    auto alt_index = value.index();
    auto encode_alt = [&](auto i_and_named_descr) {
      const auto &[i, named_descr] = i_and_named_descr;
      const auto *alt = std::get_if<i.value + 1>(&value);

      UPD_ASSERT(alt);

      named_descr.encode(*alt, ser, dest);
    };

    return updv::visit(tagged_descriptions | record_views::values | updv::enumerate, alt_index - 1, encode_alt);
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
