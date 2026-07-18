#pragma once

#include <concepts>
#include <cstddef>
#include <expected>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "../algebra/system.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../record/tags.hpp"
#include "../record/tags_of.hpp"
#include "../stream/counting_stream.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/apply.hpp"
#include "../tuple/concat.hpp"
#include "../tuple/enumerate.hpp"
#include "../tuple/find.hpp"
#include "../tuple/fold.hpp"
#include "../tuple/join.hpp"
#include "../tuple/to.hpp"
#include "../tuple/transform.hpp"
#include "../tuple/tuple_like.hpp"
#include "../tuple/typelist.hpp"
#include "../tuple/visit.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/deferred_caster.hpp"
#include "../utility/is_instance_of.hpp"
#include "../utility/template_traits.hpp"
#include "../utility/with_sequence.hpp"

namespace upd::descriptor {

template<auto Identifier, typename Rule, typename TaggedDescriptions>
struct one_of_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto size = record_size_v<TaggedDescriptions>;
  constexpr static auto alternative_types =
      decltype(std::declval<TaggedDescriptions>()
               | record_views::values
               | tuple_views::to<typelist2_t>
               | tuple_views::transform_type(
                   []<typename T> -> std::remove_cvref_t<T> {})
               | tuple_views::transform_type([]<typename T> ->
                                             typename T::value_type {})
               | tuple_views::to<typelist2_t>){};
  constexpr static auto subinput_types =
      decltype(std::declval<TaggedDescriptions>()
               | record_views::values
               | tuple_views::to<typelist2_t>
               | tuple_views::transform_type(
                   []<typename T> -> std::remove_cvref_t<T> {})
               | tuple_views::transform_type([]<typename T> ->
                                             typename T::input_type {})
               | tuple_views::to<typelist2_t>){};

  using value_type = decltype(tuple_views::apply_type(
      alternative_types, []<typename... Ts> -> std::variant<Ts...> {}));
  using input_type =
      instantiate_variadic<deferred_caster, decltype(subinput_types)>;
  using rule_type = Rule;
  using tag_type = decltype(tuple_views::apply_type(
      tags_of_v<TaggedDescriptions>
          | tuple_views::transform_type([]<typename T> ->
                                        typename T::value_type {}),
      []<typename... Ts> -> std::common_type_t<Ts...> {}));

  Rule rule;
  TaggedDescriptions tagged_descriptions;

  template<record_like Packet, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &fields, expr_t<CodecInfo>) const {
    namespace updv = upd::record_views;

    auto rule_sys =
        fields
        | updv::filter([]<auto Id, typename> { return Id != Identifier; })
        | updv::values
        | tuple_views::transform([&](const auto &field) {
            return field.rules(packet, fields, expr<CodecInfo>);
          })
        | tuple_views::join
        | tuple_views::to<std::tuple>;

    auto sys =
        tuple_views::concat(std::tuple{code_of<Identifier> = rule}, rule_sys);
    auto id = try_solve_for(code_of<Identifier>, sys);
    auto len = try_solve_for(length_of<Identifier>, sys);
    if constexpr (id == unit || len != unit) {
      return std::tuple{code_of<Identifier> = rule};
    } else {
      auto cnt_stream = counting_stream{};
      auto res =
          encode(get_or<Identifier>(packet, input_type{}), cnt_stream, sys);
      return std::tuple{code_of<Identifier> = rule,
                        length_of<Identifier> =
                            (res) ? cnt_stream.written() : 0zu};
    }
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto encode(const input_type &args,
                                      stream_interface &dest,
                                      const System &sys) const -> result<void> {
    namespace updv = upd::tuple_views;

    auto code = solve_for(code_of<Identifier>, sys | updv::to<std::tuple>);
    auto i = updv::dynfind(tags_of_v<TaggedDescriptions>, code);
    auto encode_alt = [&](const auto &named_descr) -> result<void> {
      using target_type =
          typename std::remove_cvref_t<decltype(named_descr)>::input_type;
      auto target = args.template cast_to<target_type>();

      if constexpr (is_instance_of<decltype(named_descr), description>()) {
        auto length_rule = tuple_views::fold_left(
            named_descr.m_fields
                | record_views::tags
                | updv::transform([](auto id) { return length_of<id.value>; }),
            0uz, [](auto acc, auto var) { return acc + var; });

        auto sys2 =
            updv::concat(sys, std::tuple{length_of<Identifier> = length_rule});
        if (auto res = named_descr.encode(target, dest, sys2); !res) {
          return res;
        }
      } else {
        if (auto res = named_descr.encode(target, dest, sys); !res) {
          return res;
        }
      }

      return {};
    };

    return updv::visit(tagged_descriptions | record_views::values, i,
                       encode_alt);
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, const System &sys) const -> result<value_type> {
    namespace stdr = std::ranges;
    namespace updv = upd::tuple_views;

    auto id = solve_for(code_of<Identifier>, sys);
    auto id_pos = updv::dynfind(tagged_descriptions | record_views::tags, id);

    if (id_pos == record_size_v<TaggedDescriptions>) {
      return std::unexpected{
          invalid_code_in_one_of{identifier.string, std::to_underlying(id)}};
    }

    auto make_alt = [&](const auto &id_pos_and_descr) {
      const auto &[id_pos, descr] = id_pos_and_descr;
      auto length_rule = tuple_views::fold_left(
          descr.m_fields | record_views::tags | updv::transform([](auto id) {
            return length_of<id.value>;
          }),
          0uz, [](auto acc, auto var) { return acc + var; });
      auto make_retval = [&](auto &&alt) {
        return value_type{std::in_place_index<id_pos>, UPD_FWD(alt)};
      };
      return descr
          .decode(src, updv::concat(sys, std::tuple{length_of<Identifier> =
                                                        length_rule}))
          .transform(make_retval);
    };

    return updv::visit(
        tagged_descriptions | record_views::values | updv::enumerate, id_pos,
        make_alt);
  }

  [[nodiscard]] constexpr auto bitsize(const value_type &sum_of_values) const
      noexcept(release) -> std::size_t {
    namespace updv = upd::tuple_views;
    auto alt_index = sum_of_values.index();
    if (alt_index == 0) {
      return 0;
    }

    auto seq = UPD_WITH_SEQUENCE(Is, size, &) {
      return std::tuple{expr<Is>...};
    };
    return updv::visit(seq, alt_index, [&](auto i) {
      const auto &field_value = *std::get_if<i>(&sum_of_values);
      const auto &alt_descr = upd::get_ith<i>(tagged_descriptions);
      return alt_descr.bitsize(field_value);
    });
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

  return std::move(retval);
}

} // namespace upd::descriptor
