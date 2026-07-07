#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iosfwd>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../description/codec_info.hpp"
#include "../error.hpp"
#include "../record/concat.hpp"
#include "../record/entry.hpp"
#include "../record/find.hpp"
#include "../record/fold.hpp"
#include "../record/for_each.hpp"
#include "../record/get_ith.hpp"
#include "../record/get_or.hpp"
#include "../record/instantiate.hpp"
#include "../record/name.hpp"
#include "../record/record.hpp"
#include "../record/record_like.hpp"
#include "../record/to.hpp"
#include "../record/transform.hpp"
#include "../record/values.hpp"
#include "../stream/iterator_stream.hpp"
#include "../stream/standard_stream.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/as_record.hpp"
#include "../tuple/concat.hpp"
#include "../tuple/join.hpp"
#include "../tuple/take.hpp"
#include "../tuple/to.hpp"
#include "../tuple/transform.hpp"
#include "../tuple/tuple_like.hpp"
#include "../tuple/tuple_size.hpp"
#include "../tuple/tuple_view_adaptor.hpp"
#include "../tuple/typelist.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "../utility/with_sequence.hpp"
#include "variable.hpp"

namespace upd {

template<typename T>
concept field_like = true;

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

  constexpr static auto identifiers = typelist2<Ts...>
                                      | tuple_views::transform_type([]<typename T> -> expr_t<T::identifier> {})
                                      | tuple_views::to<std::tuple>;

  explicit constexpr description(Ts... fields) : m_fields{entry{expr<fields.identifier>, std::move(fields)}...} {}

  template<record_like Args>
  constexpr void encode(const Args &args, std::ostream &dest, const char *sep) const {
    encode(args, standard_stream{nullptr, &dest, sep});
  }

  template<record_like Args, tuple_like2 System = std::tuple<>>
  constexpr void encode(const Args &args, stream_interface &dest) const {
    using namespace upd::record_views;

    auto input = input_type{};
    for_each(args, [&](auto id, const auto &value) { get<id.value>(input) = value; });
    encode(input, dest);
  }

  template<record_like Args>
  constexpr void encode(const Args &args, stream_interface &&dest) const {
    encode(args, dest);
  }

  template<tuple_like2 System = std::tuple<>>
  constexpr void encode(const input_type &input, stream_interface &dest, const System &ctx_sys = std::tuple{}) const {
    namespace updv = record_views;

    constexpr auto cdinf = codec_info{
        .operation = codec_operation::encoding,
    };

    auto rule_sys =
        m_fields
        | updv::values
        | tuple_views::transform([&](const auto &field) { return field.rules(input, m_fields, expr<cdinf>); })
        | tuple_views::join;

    auto sys = tuple_views::concat(rule_sys, ctx_sys);

    updv::for_each(m_fields, [&](auto id, const auto &field) {
      using subinput_type = typename std::remove_cvref_t<decltype(field)>::input_type;

      field.encode(get_or<id.value>(input, subinput_type{}), dest, sys);
    });
  }

  template<std::input_iterator InputIt, record_like Context = upd::record<>>
    requires std::convertible_to<std::iter_value_t<InputIt>, char>
  [[nodiscard]] constexpr auto decode(InputIt src, const Context &ctx = record{}) const {
    auto null_it = static_cast<char *>(nullptr);
    return decode(iterator_stream{src, null_it}, ctx);
  }

  template<std::input_iterator InputIt, record_like Context = upd::record<>>
    requires std::same_as<std::iter_value_t<InputIt>, std::byte>
  [[nodiscard]] constexpr auto decode(InputIt src, const Context &ctx = record{}) const {
    namespace stdr = std::ranges;
    namespace stdv = std::views;

    auto words = stdr::subrange(src, std::unreachable_sentinel) | stdv::transform(std::to_integer<char>);

    auto null_it = static_cast<char *>(nullptr);
    return decode(iterator_stream{std::begin(words), null_it}, ctx);
  }

  template<record_like Context = upd::record<>>
  [[nodiscard]] constexpr auto decode(stream_interface &src, const Context &ctx = upd::record{}) const {
    namespace updv = record_views;

    auto sys = ctx | updv::transform([](auto k, const auto &v) { return value_of<k.value> = v; }) | updv::values;

    return decode(src, sys);
  }

  template<record_like Context = upd::record<>>
  [[nodiscard]] constexpr auto decode(stream_interface &&src, const Context &ctx = upd::record{}) const {
    return decode(src, ctx);
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto decode(stream_interface &src, const System &presys) const {
    namespace updv = record_views;

    constexpr auto cdinf = codec_info{
        .operation = codec_operation::decoding,
    };

    auto err = error{};
    auto retval = result_type{};

    updv::for_each(m_fields, [&](auto id, const auto &field) {
      if (err) {
        return;
      }

      auto known_ids = identifiers | tuple_views::take_while([&]<typename Expr> { return id != Expr{}; });

      auto ctx = known_ids
                 | tuple_views::transform([&]<auto Id>(expr_t<Id>) { return keyword2<Id>{} = get<Id>(retval); })
                 | tuple_views::as_record
                 | updv::to<upd::record>;

      auto rules = tuple_views::concat(known_ids, std::tuple{id})
                   | tuple_views::to<std::tuple>
                   | tuple_views::transform([&]<auto Id>(expr_t<Id>) { return keyword2<Id>{} = get<Id>(m_fields); })
                   | tuple_views::as_record
                   | updv::to<upd::record>
                   | updv::transform([&](auto, const auto &field) { return field.rules(ctx, m_fields, expr<cdinf>); })
                   | updv::to<upd::record>
                   | updv::values
                   | tuple_views::join
                   | tuple_views::to<std::tuple>;

      auto sys = tuple_views::concat(presys, rules);
      auto maybe_field_value = field.decode(src, m_fields, sys);
      if (!maybe_field_value) {
        err = maybe_field_value.error();
        return;
      }

      get<id.value>(retval) = *maybe_field_value;
    });

    return result_if_no_error(std::move(retval), std::move(err));
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

constexpr auto empty_description = description<>{};

} // namespace upd
