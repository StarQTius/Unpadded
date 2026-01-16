#pragma once

#include <concepts>
#include <expected>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "../constexpr.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../record/tags.hpp"
#include "../record/tags_of.hpp"
#include "../stream_interface.hpp"
#include "../tuple/apply.hpp"
#include "../tuple/concat.hpp"
#include "../tuple/enumerate.hpp"
#include "../tuple/find.hpp"
#include "../tuple/to.hpp"
#include "../tuple/transform.hpp"
#include "../tuple/tuple_like.hpp"
#include "../tuple/typelist.hpp"
#include "../tuple/visit.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

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

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto make_value(const System &, value_type v) const -> value_type {
    return v;
  }

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

  template<record_like Packet, record_like Fields, serializer Serializer>
  [[nodiscard]] constexpr auto rules(const Packet &, const Fields &, Serializer &) const {
    return std::tuple{code_of<Identifier> = rule};
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
      auto lensys = std::tuple{length_of<Identifier> = descr.length()};
      auto make_retval = [&](auto &&alt) { return value_type{std::in_place_index<id_pos + 1>, UPD_FWD(alt)}; };
      auto retval = descr.decode(src, ser, packet, updv::concat(sys, lensys)).transform(make_retval);
      return retval;
    };

    return updv::visit(tagged_descriptions | record_views::values | updv::enumerate, id_pos, make_alt);
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr void encode(const value_type &value, Serializer &ser, stream_interface &dest, const System &sys) const
      noexcept(release) {
    namespace updv = upd::tuple_views;

    auto alt_index = value.index();
    auto encode_alt = [&](auto i_and_named_descr) {
      const auto &[i, named_descr] = i_and_named_descr;
      const auto *alt = std::get_if<i.value + 1>(&value);

      UPD_ASSERT(alt);

      named_descr.encode(*alt, ser, dest, sys);
    };

    return updv::visit(tagged_descriptions | record_views::values | updv::enumerate, alt_index - 1, encode_alt);
  }

  [[nodiscard]] constexpr static auto length() noexcept(release) { return length_of<Identifier>; }
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
