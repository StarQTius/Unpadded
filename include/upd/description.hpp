#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iosfwd>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

#include "algebra/side.hpp"
#include "algebra/variable.hpp"
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
#include "stream_interface.hpp"
#include "tuple/as_record.hpp"
#include "tuple/concat.hpp"
#include "tuple/join.hpp"
#include "tuple/transform.hpp"
#include "tuple/tuple_like.hpp"
#include "tuple/tuple_size.hpp"
#include "tuple/tuple_view_adaptor.hpp"
#include "tuple/typelist.hpp"
#include "upd.hpp"
#include "with_sequence.hpp"

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

} // namespace upd

namespace upd {

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
concept decodable_field = serializer<Serializer>
                          && deducible_field<T, Serializer>
                          && requires(T x, Serializer ser, const record<> packet, const byte_type<Serializer> *src) {
                               { x.decode(src, ser, packet) } -> std::same_as<typename T::value_type>;
                             };

template<typename T, typename Serializer>
concept encodable_field = serializer<Serializer>
                          && deducible_field<T, Serializer>
                          && requires(T x, Serializer ser, typename T::value_type value, byte_type<Serializer> *dest) {
                               { x.encode(value, ser, dest) } -> std::same_as<void>;
                             };

template<field_like... Ts>
class description {
  template<typename... _Ts, typename... Us>
  friend constexpr auto operator|(description<_Ts...> lhs, description<Us...> rhs) noexcept(release);

public:
  using result_type =
      decltype(typelist2<Ts...>
               | tuple_views::transform_type([]<typename T> -> entry<T::identifier, typename T::value_type> {})
               | tuple_views::as_record
               | record_views::instantiate<record>);

  using storage_type = decltype(typelist2<Ts...>
                                | tuple_views::transform_type([]<typename T> -> entry<T::identifier, T> {})
                                | tuple_views::as_record
                                | record_views::instantiate<record>);

  using input_type =
      decltype(typelist2<Ts...>
               | tuple_views::transform_type([]<typename T> -> entry<T::identifier, typename T::input_type> {})
               | tuple_views::as_record
               | record_views::instantiate<record>);

  explicit constexpr description(Ts... fields) : m_fields{entry{expr<fields.identifier>, std::move(fields)}...} {}

  template<record_like Args, serializer Serializer>
  constexpr void encode(const Args &args, Serializer &ser, std::ostream &dest, const char *sep) const {
    encode(args, ser, standard_stream{nullptr, &dest, sep});
  }

  template<record_like Args, serializer Serializer, tuple_like2 System = std::tuple<>>
  constexpr void
  encode(const Args &args, Serializer &ser, stream_interface &dest, const System &ctx_sys = std::tuple{}) const {
    namespace updv = record_views;

    auto rule_sys = m_fields
                    | updv::values
                    | tuple_views::transform([&](const auto &field) { return field.rules(args, m_fields, ser); })
                    | tuple_views::join;

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
            return updv::concat(retval | updv::take_until<id.value>, entry{id, get<id.value>(ctx)})
                   | updv::to<upd::record>;
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
          m_fields
          | updv::transform([&](auto, const auto &field) { return field.deduce(retval, ser, m_fields, sys); })
          | updv::join([](auto, auto k) { return k; })
          | updv::to<record>;

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

  template<record_like NamedFieldValues>
  [[nodiscard]] constexpr auto bitsize(const NamedFieldValues &named_field_values) const noexcept(release)
      -> std::size_t {
    namespace updv = upd::record_views;
    return updv::fold_left(named_field_values, 0uz, [&](std::size_t acc, auto k, const auto &field_value) {
      auto field_pos = updv::find_if(m_fields, [&](auto id, const auto &) { return expr<id == k>; });
      return acc + get_ith<field_pos>(m_fields).bitsize(field_value.value());
    });
  }

  template<typename... Entries>
  [[nodiscard]] constexpr auto bitsize(const record<Entries...> &named_field_values) const noexcept(release)
      -> std::size_t {
    namespace updv = record_views;
    return updv::fold_left(named_field_values, 0uz, [&](std::size_t acc, auto k, const auto &field_value) {
      auto field_pos = updv::find_if(m_fields, [&](auto id, auto) { return expr<id == k>; });
      return acc + get_ith<field_pos>(m_fields).bitsize(field_value);
    });
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

} // namespace upd::descriptor

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
