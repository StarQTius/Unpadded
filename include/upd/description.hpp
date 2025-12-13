#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "constexpr.hpp"
#include "error.hpp"
#include "functional.hpp"
#include "get.hpp"
#include "is_instance_of.hpp"
#include "named_value.hpp"
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
#include "record/record_like.hpp"
#include "record/to.hpp"
#include "record/transform.hpp"
#include "record/values.hpp"
#include "record/zip.hpp"
#include "safe_operation.hpp"
#include "static_vector.hpp"
#include "stream_interface.hpp"
#include "template_traits.hpp"
#include "token.hpp"
#include "tuple.hpp"
#include "tuple/apply.hpp"
#include "tuple/as_record.hpp"
#include "tuple/concat.hpp"
#include "tuple/enumerate.hpp"
#include "tuple/fold.hpp"
#include "tuple/reverse.hpp"
#include "tuple/to.hpp"
#include "tuple/transform.hpp"
#include "tuple/tuple_element.hpp"
#include "tuple/tuple_size.hpp"
#include "tuple/tuple_view_adaptor.hpp"
#include "tuple/typelist.hpp"
#include "tuple/visit.hpp"
#include "tuple/zip.hpp"
#include "tuple_impl.hpp"
#include "typelist.hpp"
#include "upd.hpp"
#include "with_sequence.hpp"

namespace upd {

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

template<typename NamedValue, typename Field>
  requires(is_instance_of<NamedValue, named_value>())
[[nodiscard]] constexpr auto bitsize(const NamedValue &nv, const Field &field) noexcept(release) -> std::size_t {
  return bitsize(nv.value(), field);
}

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
  return updv::visit(sequence<sizeof...(Ts) - 1>, alt_index - 1, [&](auto i) {
    const auto &field_value = *std::get_if<i + 1>(&sum_of_field_values);
    const auto &alt_descr = field.tagged_descriptions[i];
    return bitsize(field_value, alt_descr);
  });
}

template<names Identifiers, typename... Ts, typename Description>
[[nodiscard]] constexpr auto bitsize(const named_tuple<Identifiers, Ts...> &named_field_values,
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

template<name Identifier>
constexpr auto value_of = field_expression_t{
    expr<Identifier>, [](auto &packet, const auto &) -> auto & { return get<Identifier>(packet); }, {}};

template<name Identifier>
constexpr auto length_of = field_expression_t{
    expr<Identifier>,
    [](const auto &packet, const auto &fields) {
      namespace updv = upd::record_views;

      auto field_pos = updv::find_if(fields, [](auto id, const auto &) { return expr<id == Identifier>; });
      return static_cast<std::uint16_t>(bitsize(get<Identifier>(packet), get_ith<field_pos>(fields)));
    },
    {}};

template<name Identifier>
constexpr auto code_of = field_expression_t{
    expr<Identifier>, [](const auto &packet, const auto &) { return packet[expr<Identifier>].index(); }, {}};

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
  constexpr static auto identifiers = typelist2<Ts...> |
                                      tuple_views::transform_type([]<typename T> -> expr_t<T::identifier> {}) |
                                      tuple_views::to<names>;

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

    auto packet = m_fields | updv::transform([&]<typename Field>(auto, const Field &field) {
                    auto id = keyword2<field.identifier>{};
                    if constexpr (has_tag<field.identifier>(args)) {
                      return field.make_value(args[id]);
                    } else {
                      return field.default_value();
                    }
                  }) |
                  updv::to<record>;

    updv::for_each(m_fields, [&](auto, const auto &field) {
      updv::for_each(field.deduce(packet, ser, m_fields),
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

  template<std::input_iterator InputIt, serializer Serializer, record_like Context>
    requires std::convertible_to<std::iter_value_t<InputIt>, word_t>
  [[nodiscard]] constexpr auto decode(InputIt src, Serializer &ser, const Context &ctx = record{}) const {
    auto null_it = static_cast<word_t *>(nullptr);
    return decode(iterator_stream{src, null_it}, ser, ctx);
  }

  template<std::input_iterator InputIt, serializer Serializer, record_like Context>
    requires std::same_as<std::iter_value_t<InputIt>, std::byte>
  [[nodiscard]] constexpr auto decode(InputIt src, Serializer &ser, const Context &ctx = record{}) const {
    namespace stdr = std::ranges;
    namespace stdv = std::views;

    auto words = stdr::subrange(src, std::unreachable_sentinel) | stdv::transform(std::to_integer<word_t>);

    auto null_it = static_cast<word_t *>(nullptr);
    return decode(iterator_stream{std::begin(words), null_it}, ser, ctx);
  }

  template<serializer Serializer, record_like Context>
  [[nodiscard]] constexpr auto decode(stream_interface &src, Serializer &ser, const Context &ctx = record{}) const {
    namespace updv = record_views;

    auto err = error{};
    auto retval =
        m_fields | updv::transform([](auto, const auto &field) { return field.default_value(); }) | updv::to<record>;

    if (!err) {
      updv::for_each(m_fields, [&](auto id, const auto &field) {
        ser.checkpoint(id.value.string);
        auto packet = updv::concat(std::as_const(retval), ctx);
        auto maybe_field_value = field.decode(src, ser, packet, m_fields);
        if (maybe_field_value) {
          get<id.value>(retval) = *maybe_field_value;
        } else {
          err = maybe_field_value.error();
        }
      });
    }

    if (!err) {
      auto merged = updv::concat(std::as_const(retval), ctx);
      auto deduced = m_fields |
                     updv::transform([&](auto, const auto &field) { return field.deduce(retval, ser, m_fields); }) |
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

  template<serializer Serializer, record_like Context>
  [[nodiscard]] constexpr auto decode(stream_interface &&src, Serializer &ser, const Context &ctx = record{}) const {
    return decode(src, ser, ctx);
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

template<name Identifier, bool Signedness, std::size_t Width>
struct field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type(UPD_FWD(args)...);
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
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
      return ser.serialize_signed(value, upd::width<width>, dest);
    } else {
      return ser.serialize_unsigned(value, upd::width<width>, dest);
    }
  }
};

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

  using value_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }
  using rule_type = Rule;

  rule_type rule;

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr auto deduce(Packet &packet, Serializer &, const Fields &fields) const {
    return tagged_tuple{named_value{expr<identifier>, rule.deduce(std::as_const(packet), fields)}};
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
      return ser.serialize_signed(value, upd::width<width>, dest);
    } else {
      return ser.serialize_unsigned(value, upd::width<width>, dest);
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

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr auto deduce(Packet &packet, Serializer &, const Fields &fields) const {
    return tagged_tuple{named_value{expr<identifier>, rule.deduce(std::as_const(packet), fields)}};
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

  using value_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
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

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
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

template<name Identifier, std::size_t Width>
struct constant_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = std::intmax_t;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  value_type field_value;

  [[nodiscard]] constexpr auto default_value() const noexcept(release) -> value_type { return field_value; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
  }

  template<serializer Serializer, record_like Packet, record_like Fields>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Packet &, const Fields &)
      -> result<value_type> {
    return ser.deserialize_unsigned(src, upd::width<width>);
  }

  template<serializer Serializer>
  constexpr static void encode(value_type value, Serializer &ser, stream_interface &dest) {
    return ser.serialize_unsigned(value, upd::width<width>, dest);
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

  using value_type = std::uintmax_t;

  template<typename... Args>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type(UPD_FWD(args)...);
  }

  BinaryOp op;
  value_type init;
  FieldFilter identifier_filter;

  [[nodiscard]] constexpr auto default_value() const noexcept(release) -> value_type { return init; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr auto deduce(Packet &packet, Serializer &ser, const Fields &fields) const {
    using namespace upd::literals;
    namespace updv = upd::record_views;

    struct stream_t : stream_interface {
      stream_t(const BinaryOp *op, value_type acc) : op{op}, acc{acc} {}

      auto read(std::size_t, word_t *) -> stream_error_t override { return 1; }

      auto write(const word_t *src, std::size_t size) -> stream_error_t override {
        namespace stdr = std::ranges;

        for (auto w : stdr::subrange{src, src + size}) {
          acc = UPD_INVOKE(*op, acc, static_cast<value_type>(w));
        }

        return 0;
      }

      const BinaryOp *op;
      value_type acc;
    } dest{&op, init};

    auto field_filter = [&]<auto Id>(expr_t<Id>, auto) { return UPD_INVOKE(FieldFilter{}, expr<Id>); };

    updv::for_each(updv::zip(packet | updv::filter(field_filter), fields), [&](auto, const auto &value_and_field) {
      const auto &[value, field] = value_and_field;
      field.encode(value, ser, dest);
    });

    return tagged_tuple{named_value{expr<identifier>, dest.acc}};
  }

  template<serializer Serializer, record_like Packet, record_like Fields>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Packet &, const Fields &)
      -> result<value_type> {
    return ser.deserialize_unsigned(src, upd::width<width>);
  }

  template<serializer Serializer>
  constexpr static void encode(value_type value, Serializer &ser, stream_interface &dest) {
    return ser.serialize_unsigned(value, upd::width<width>, dest);
  }
};

template<name Identifier, typename BinaryOp, std::size_t Width>
[[nodiscard]] constexpr auto
checksum(BinaryOp op, std::uintmax_t init, width_t<Width>, all_fields_t) noexcept(release) {
  auto is_not_this_field = [](auto id) { return expr<id != Identifier>; };

  auto retval =
      checksum_t<Identifier, BinaryOp, Width, decltype(is_not_this_field)>{std::move(op), init, is_not_this_field};

  return description{std::move(retval)};
}

template<auto Identifier, typename Rule, typename TaggedDescriptions>
struct one_of_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto size = std::tuple_size_v<TaggedDescriptions>;
  constexpr static auto alternative_types =
      decltype(tuple_views::concat(typelist2<upd::description<>>,
                                   std::declval<TaggedDescriptions>() | tuple_views::to<typelist2_t>) |
               tuple_views::transform_type([]<typename T> -> std::remove_cvref_t<T> {}) |
               tuple_views::transform_type([]<typename T> -> typename T::result_type {}) |
               tuple_views::to<typelist2_t>){};

  using value_type = decltype(tuple_views::apply_type(alternative_types, []<typename... Ts> -> std::variant<Ts...> {}));

  using tag_type = decltype(tuple_views::apply_type(
      TaggedDescriptions::identifiers | tuple_views::transform_type([]<typename T> -> typename T::value_type {}),
      []<typename... Ts> -> std::common_type_t<Ts...> {}));

  template<typename... Args>
    requires std::constructible_from<value_type, Args...>
  [[nodiscard]] constexpr auto make_value(Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  template<typename Choice>
    requires(is_instance_of<Choice, choice_t>())
  [[nodiscard]] constexpr auto make_value(Choice &&ch) const -> value_type {
    auto id_pos = tagged_descriptions.identifiers.find(expr<ch.code>);
    using alt_type = tuple_element_t<id_pos + 1, decltype(alternative_types)>;

    return UPD_FWD(ch).arguments.apply(
        [&](auto &&...args) { return value_type{std::in_place_index<id_pos + 1>, alt_type(UPD_FWD(args)...)}; });
  }

  using rule_type = Rule;

  rule_type rule;
  TaggedDescriptions tagged_descriptions;

  [[nodiscard]] constexpr static auto default_value() -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr auto deduce(const Packet &packet, Serializer &, const Fields &) const noexcept(release) {
    namespace updv = upd::tuple_views;

    auto id_pos = get<identifier>(packet).index() - 1;
    auto id = updv::visit(tagged_descriptions.identifiers, id_pos, [&](auto id) -> tag_type { return id; });
    return tagged_tuple{keyword<rule_type::from_identifier>{} = UPD_INVOKE(inverse(rule.chain), id)};
  }

  template<serializer Serializer, record_like Packet, record_like Fields>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, Serializer &ser, const Packet &packet, const Fields &fields) const
      -> result<value_type> {
    namespace stdr = std::ranges;
    namespace updv = upd::tuple_views;

    auto id = rule.deduce(packet, fields);
    auto id_pos = tagged_descriptions.identifiers.find(id);

    if (id_pos == tagged_descriptions.size()) {
      return std::unexpected{invalid_code_in_one_of{identifier.string, std::to_underlying(id)}};
    }

    auto make_alt = [&](const auto &id_pos_and_descr) {
      const auto &[id_pos, descr] = id_pos_and_descr;
      auto make_retval = [&](auto &&alt) { return value_type{std::in_place_index<id_pos + 1>, UPD_FWD(alt)}; };
      auto retval = descr.decode(src, ser, packet).transform(make_retval);
      return retval;
    };

    return updv::visit(updv::zip(sequence<size>, tagged_descriptions), id_pos, make_alt);
  }

  template<serializer Serializer>
  constexpr void encode(const value_type &value, Serializer &ser, stream_interface &dest) const noexcept(release) {
    namespace updv = upd::tuple_views;

    auto alt_index = value.index();
    auto encode_alt = [&](const auto &i_and_named_descr) {
      const auto &[i, named_descr] = i_and_named_descr;
      const auto *alt = std::get_if<i.value + 1>(&value);

      UPD_ASSERT(alt);

      named_descr.encode(*alt, ser, dest);
    };

    return updv::visit(tagged_descriptions | updv::enumerate, alt_index - 1, encode_alt);
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

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return named_tuple{};
  }

  template<serializer Serializer, record_like Packet, record_like Fields>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, Serializer &ser, const Packet &packet, const Fields &fields) const
      -> result<value_type> {
    namespace stdv = std::views;

    auto count = rule.deduce(packet, fields);
    if (count < 0) {
      return std::unexpected{negative_repetition_count{identifier.string, static_cast<std::intmax_t>(count)}};
    }
    if (Max < count) {
      return std::unexpected{repeated_beyond_max{identifier.string, static_cast<std::uintmax_t>(count), Max}};
    }

    auto retval = static_vector<typename Description::result_type, Max>{};

    for (auto _ : stdv::iota(0uz, std::size_t(count))) {
      auto maybe_value = description.decode(src, ser, packet);
      if (!maybe_value) {
        return std::unexpected{std::move(maybe_value).error()};
      }
      retval.push_back(std::move(maybe_value).value());
    }

    return retval;
  }

  template<serializer Serializer>
  constexpr void encode(const value_type &value, Serializer &ser, stream_interface &dest) const {
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
