#pragma once

#include <concepts>
#include <cstddef>
#include <expected>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "../algebra/system.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../record/tags_of.hpp"
#include "../stream/counting_stream.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/apply.hpp"
#include "../tuple/concat.hpp"
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

template<typename Rule, typename Arms>
struct one_of_t {
  constexpr static auto size = record_size_v<Arms>;
  constexpr static auto arm_types =
      decltype(std::declval<Arms>()
               | record_views::values
               | tuple_views::to<typelist2_t>
               | tuple_views::transform_type([]<typename T> ->
                                             typename T::value_type {})
               | tuple_views::to<typelist2_t>){};
  constexpr static auto subinput_types =
      decltype(std::declval<Arms>()
               | record_views::values
               | tuple_views::to<typelist2_t>
               | tuple_views::transform_type([]<typename T> ->
                                             typename T::input_type {})
               | tuple_views::to<typelist2_t>){};

  using value_type = decltype(tuple_views::apply_type(
      arm_types, []<typename... Ts> -> std::variant<Ts...> {}));
  using input_type =
      instantiate_variadic<deferred_caster, decltype(subinput_types)>;
  using rule_type = Rule;
  using tag_type = decltype(tuple_views::apply_type(
      tags_of_v<Arms>
          | tuple_views::transform_type([]<typename T> ->
                                        typename T::value_type {}),
      []<typename... Ts> -> std::common_type_t<Ts...> {}));

  Rule rule;
  Arms arms;

  template<auto Id,
           record_like Packet,
           record_like Fields,
           codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &fields, expr_t<CodecInfo>) const {
    using namespace upd::record_views;

    auto other_rules =
        fields
        | filter([]<auto K, typename> { return K != Id; })
        | transform([&]<auto K>(const auto &field) {
            return field.template rules<K>(packet, fields, expr<CodecInfo>);
          })
        | values
        | tuple_views::join
        | tuple_views::to<std::tuple>;

    auto sys = tuple_views::concat(std::tuple{code_of<Id> = rule}, other_rules);
    auto id = try_solve_for(code_of<Id>, sys);
    auto len = try_solve_for(length_of<Id>, sys);
    if constexpr (id == unit || len != unit) {
      return std::tuple{code_of<Id> = rule};
    } else {
      auto cnt_stream = counting_stream{};
      auto res = encode<Id>(get_or<Id>(packet, input_type{}), cnt_stream, sys);
      auto count = (res) ? cnt_stream.written() : 0zu;
      return std::tuple{code_of<Id> = rule, length_of<Id> = count};
    }
  }

  template<auto Id, tuple_like2 System>
  [[nodiscard]] constexpr auto encode(const input_type &args,
                                      stream_interface &dest,
                                      const System &sys) const -> result<void> {
    using namespace upd::tuple_views;

    auto encode_descr = [&]<typename D>(const D &descr, const auto &in) {
      auto lengths = D::identifiers
                     | transform([](auto id) { return length_of<id.value>; });

      auto length_sum = fold_left(lengths, 0uz, std::plus<>{});
      auto length_rule = std::tuple{length_of<Id> = length_sum};
      auto full_sys = concat(sys, length_rule);
      return descr.template encode<Id>(in, dest, full_sys);
    };

    auto encode_codec = [&](const auto &codec, const auto &in) {
      return codec.template encode<Id>(in, dest, sys);
    };

    auto encode_alt = [&]<auto, typename Arm>(const Arm &arm) {
      using arm_input_type = typename Arm::input_type;
      auto in = args.template cast_to<arm_input_type>();
      if constexpr (is_instance_of<Arm, description>()) {
        return encode_descr(arm, in);
      } else {
        return encode_codec(arm, in);
      }
    };

    auto code = solve_for(code_of<Id>, sys);
    return record_views::visit(arms, code, encode_alt);
  };

  template<auto Id, tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, const System &sys) const -> result<value_type> {
    using namespace upd::tuple_views;

    auto k = solve_for(code_of<Id>, sys);
    auto make_alt = [&]<auto K>(const auto &descr) -> result<value_type> {
      constexpr auto i = dynfind(tags_of_v<Arms>, K);
      if (i == record_size_v<Arms>) {
        return std::unexpected{
            invalid_code_in_one_of{Id.string, std::to_underlying(k)}};
      }

      auto lengths = descr.identifiers
                     | transform([](auto id) { return length_of<id.value>; });

      auto length_sum = fold_left(lengths, 0uz, std::plus<>{});
      auto length_rule = std::tuple{length_of<Id> = length_sum};
      auto full_sys = concat(sys, length_rule);
      auto maybe_alt = descr.template decode<Id>(src, full_sys);
      if (!maybe_alt) {
        return std::unexpected{maybe_alt.error()};
      }

      return value_type{std::in_place_index<i>, UPD_FWD(*maybe_alt)};
    };

    return record_views::visit(arms, k, make_alt);
  }

  [[nodiscard]] constexpr auto bitsize(const value_type &sum_of_values) const
      noexcept(release) -> std::size_t {
    using namespace upd::tuple_views;

    auto i = sum_of_values.index();
    auto seq = UPD_WITH_SEQUENCE(Is, size, &) {
      return std::tuple{expr<Is>...};
    };

    return visit(seq, i, [&](auto i) {
      const auto &value = *std::get_if<i>(&sum_of_values);
      const auto &descr = upd::get_ith<i>(arms);
      return descr.bitsize(value);
    });
  }
};

template<typename Rule, typename... WhenThens>
[[nodiscard]] constexpr auto one_of(Rule &&rule, WhenThens &&...when_thens) {
  using namespace upd::record_views;

  auto arms = aggregate_when_thens(UPD_FWD(when_thens)...)
              | transform([]<auto, typename T>(T &&v) -> decltype(auto) {
                  if constexpr (std::same_as<std::remove_cvref_t<T>, unit_t>) {
                    return description<>{};
                  } else if constexpr (record_like<T>) {
                    return description{std::move(v)};
                  } else {
                    return std::move(v);
                  }
                })
              | to<record>;

  using rule_type = std::remove_cvref_t<Rule>;
  using arm_types = decltype(arms);
  return one_of_t<rule_type, arm_types>{
      .rule = UPD_FWD(rule),
      .arms = std::move(arms),
  };
}

} // namespace upd::descriptor
