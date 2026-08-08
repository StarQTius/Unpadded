#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <expected>
#include <iosfwd>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../algebra/system.hpp"
#include "../description/codec_info.hpp"
#include "../error.hpp"
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
#include "../tuple/filter.hpp"
#include "../tuple/join.hpp"
#include "../tuple/take.hpp"
#include "../tuple/to.hpp"
#include "../tuple/transform.hpp"
#include "../tuple/tuple_like.hpp"
#include "../tuple/tuple_view_adaptor.hpp"
#include "../tuple/typelist.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/equivalent_to.hpp"
#include "../utility/get.hpp"
#include "../utility/is_instance_of.hpp"
#include "codec.hpp"
#include "variable.hpp"

namespace upd {

template<typename...>
class description;

template<auto... Identifiers, typename... Fields>
class description<entry<Identifiers, Fields>...> {
public:
  using result_type =
      decltype(typelist2<entry<Identifiers, typename Fields::value_type>...>
               | tuple_views::as_record
               | record_views::instantiate<record>);

  using value_type =
      decltype(typelist2<entry<Identifiers, typename Fields::value_type>...>
               | tuple_views::filter([]<typename Entry> {
                   return !std::same_as<typename Entry::value_type, unit_t>;
                 })
               | tuple_views::as_record
               | record_views::instantiate<record>);

  using storage_type = decltype(typelist2<entry<Identifiers, Fields>...>
                                | tuple_views::as_record
                                | record_views::instantiate<record>);

  using input_type =
      decltype(typelist2<entry<Identifiers, typename Fields::input_type>...>
               | tuple_views::as_record
               | record_views::instantiate<record>);

  constexpr static auto identifiers = std::tuple{expr<Identifiers>...};

  constexpr description()
    requires(std::default_initializable<Fields> && ...)
  = default;

  template<typename... Entries>
    requires((is_instance_of<Entries, entry>() && ...)
             && sizeof...(Entries)
             == sizeof...(Fields)
             && sizeof...(Fields)
             > 0)
  explicit constexpr description(Entries &&...es) : m_fields{UPD_FWD(es)...} {}

  template<record_like Record>
  explicit constexpr description(Record &&rec) : m_fields{UPD_FWD(rec)} {}

  template<auto = unit,
           record_like Packet,
           record_like FieldRecord,
           codec_info CodecInfo>
  [[nodiscard]] constexpr static auto
  rules(const Packet &, const FieldRecord &, expr_t<CodecInfo>) {
    return std::tuple{};
  }

  template<auto Id = unit, record_like Args>
  [[nodiscard]]
  constexpr auto
  encode(const Args &args, std::ostream &dest, const char *sep) const
      -> result<void> {
    return encode<Id>(args, standard_stream{nullptr, &dest, sep});
  }

  template<auto Id = unit, record_like Args>
  [[nodiscard]]
  constexpr auto
  encode(const Args &args, stream_interface &dest) const -> result<void> {
    using namespace upd::record_views;

    auto input = input_type{};
    for_each(args, [&]<auto K>(const auto &value) { get<K>(input) = value; });
    return encode<Id>(input, dest);
  }

  template<auto Id = unit, record_like Args>
  [[nodiscard]]
  constexpr auto
  encode(const Args &args, stream_interface &&dest) const -> result<void> {
    return encode<Id>(args, dest);
  }

  template<auto Id = unit>
  [[nodiscard]]
  constexpr auto encode(const input_type &input, stream_interface &dest) const
      -> result<void> {
    return encode<Id>(input, dest, std::tuple{});
  }

  template<auto = unit, tuple_like2 System>
  [[nodiscard]]
  constexpr auto encode(const input_type &input,
                        stream_interface &dest,
                        const System &ctx_sys) const -> result<void> {
    namespace updv = record_views;

    constexpr auto cdinf = codec_info{
        .operation = codec_operation::encoding,
    };

    auto res = result<void>{};
    auto rule_sys =
        m_fields
        | updv::transform([&]<auto Id>(const auto &field) {
            return field.template rules<Id>(input, m_fields, expr<cdinf>);
          })
        | updv::values
        | tuple_views::join;

    auto sys = tuple_views::concat(rule_sys, ctx_sys);

    updv::for_each(m_fields, [&]<auto Id>(const auto &field) {
      using subinput_type =
          typename std::remove_cvref_t<decltype(field)>::input_type;
      if (!res) {
        return;
      }

      res = field.template encode<Id>(get_or<Id>(input, subinput_type{}), dest,
                                      sys);
    });

    return res;
  }

  template<auto Id = unit,
           std::input_iterator InputIt,
           record_like Context = upd::record<>>
    requires std::convertible_to<std::iter_value_t<InputIt>, char>
  [[nodiscard]] constexpr auto
  decode(InputIt src, const Context &ctx = record{}) const {
    auto null_it = static_cast<char *>(nullptr);
    return decode<Id>(iterator_stream{src, null_it}, ctx);
  }

  template<auto Id = unit,
           std::input_iterator InputIt,
           record_like Context = upd::record<>>
    requires std::same_as<std::iter_value_t<InputIt>, std::byte>
  [[nodiscard]] constexpr auto
  decode(InputIt src, const Context &ctx = record{}) const {
    namespace stdr = std::ranges;
    namespace stdv = std::views;

    auto words = stdr::subrange(src, std::unreachable_sentinel)
                 | stdv::transform(std::to_integer<char>);

    auto null_it = static_cast<char *>(nullptr);
    return decode<Id>(iterator_stream{std::begin(words), null_it}, ctx);
  }

  template<auto Id = unit, record_like Context = upd::record<>>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, const Context &ctx = upd::record{}) const {
    namespace updv = record_views;

    auto sys =
        ctx
        | updv::transform([]<auto K>(const auto &v) { return value_of<K> = v; })
        | updv::values;

    return decode<Id>(src, sys);
  }

  template<auto Id = unit, record_like Context = upd::record<>>
  [[nodiscard]] constexpr auto
  decode(stream_interface &&src, const Context &ctx = upd::record{}) const {
    return decode<Id>(src, ctx);
  }

  template<auto = unit, tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, const System &presys) const
      -> result<value_type> {
    namespace updv = record_views;

    constexpr auto cdinf = codec_info{
        .operation = codec_operation::decoding,
    };

    auto maybe_retval = result<value_type>{};

    updv::for_each(m_fields, [&]<auto Id>(const auto &field) {
      if (!maybe_retval) {
        return;
      }

      auto known_ids =
          identifiers | tuple_views::take_while([&]<typename Expr> {
            return expr<Id != Expr::value>;
          });

      auto ctx = known_ids
                 | tuple_views::transform([&]<auto K>(expr_t<K>) {
                     return keyword2<K>{} = get_or<K>(*maybe_retval, unit);
                   })
                 | tuple_views::as_record
                 | updv::to<upd::record>;

      auto rules =
          tuple_views::concat(known_ids, std::tuple{expr<Id>})
          | tuple_views::to<std::tuple>
          | tuple_views::transform([&]<auto K>(expr_t<K>) {
              return keyword2<K>{} = get<K>(m_fields);
            })
          | tuple_views::as_record
          | updv::to<upd::record>
          | updv::transform([&]<auto K>(const auto &field) {
              return field.template rules<K>(ctx, m_fields, expr<cdinf>);
            })
          | updv::to<upd::record>
          | updv::values
          | tuple_views::join
          | tuple_views::to<std::tuple>;

      auto sys = tuple_views::concat(presys, rules);
      if (auto maybe_value = field.template decode<Id>(src, sys); maybe_value) {
        get_or<Id>(*maybe_retval, std::ignore) = *std::move(maybe_value);
      } else {
        maybe_retval = std::unexpected{maybe_value.error()};
      }
    });

    return maybe_retval;
  }

  [[nodiscard]] constexpr auto
  bitsize(const value_type &packet) const noexcept(release) -> std::size_t {
    namespace updv = record_views;
    return updv::fold_left(
        packet, 0uz, [&]<auto K>(std::size_t acc, const auto &field_value) {
          auto field_pos = updv::find_if(
              m_fields, []<auto Id, typename> { return equivalent_to<Id, K>; });
          return acc + get_ith<field_pos>(m_fields).bitsize(field_value);
        });
  }

  storage_type m_fields;
};

template<auto... Identifiers, codec... Fields>
explicit description(entry<Identifiers, Fields>...)
    -> description<entry<Identifiers, Fields>...>;

template<auto... Identifiers, codec... Fields>
explicit description(record<entry<Identifiers, Fields>...>)
    -> description<entry<Identifiers, Fields>...>;

} // namespace upd
