#pragma once

#include <concepts>
#include <cstddef>
#include <expected>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "../algebra/system.hpp"
#include "../constexpr.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../is_instance_of.hpp"
#include "../record.hpp"
#include "../record/tags.hpp"
#include "../record/tags_of.hpp"
#include "../static_assert.hpp"
#include "../stream_interface.hpp"
#include "../template_traits.hpp"
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
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "serializer.hpp"

namespace upd {

template<typename... Targets>
class deferred_caster {
  template<typename Orig, typename Target>
  static auto caster(const void *orig) noexcept(release) -> Target {
    const auto &o = *reinterpret_cast<const Orig *>(orig);
    auto cast_alt = []<typename T>(const T &alt) -> Target {
      if constexpr (std::constructible_from<Target, T>) {
        return Target{alt};
      } else {
        UPD_ASSERT(false);
      }
    };

    if constexpr (std::constructible_from<Target, Orig>) {
      return Target{o};
    } else if constexpr (is_instance_of<Orig, std::variant>()) {
      return std::visit(cast_alt, o);
    } else {
      UPD_ASSERT(false);
    }
  }

public:
  template<typename T>
  constexpr deferred_caster(const T &orig) noexcept(release) : m_orig{&orig}, m_casters{caster<T, Targets>...} {}

  template<typename Target>
  [[nodiscard]] constexpr auto cast_to() const noexcept(release) {
    using namespace tuple_views;

    auto i = find_if(m_casters, []<typename F>(typebox<F>) { return std::is_invocable_r_v<Target, F, const void *>; });
    UPD_STATIC_ASSERT(i < sizeof...(Targets), "Cannot cast to target type `{}`", typebox<Target>{});

    return UPD_INVOKE(get<i>(m_casters), m_orig);
  }

private:
  const void *m_orig;
  std::tuple<Targets (*)(const void *)...> m_casters;
};

class counting_stream : public stream_interface {
public:
  counting_stream() = default;

  [[nodiscard]] auto read(std::size_t count, word_t *) noexcept(release) -> stream_error_t override {
    m_read += count;
    return 0;
  }

  [[nodiscard]] auto write(const word_t *, std::size_t size) noexcept(release) -> stream_error_t override {
    m_written += size;
    return 0;
  }

  [[nodiscard]] constexpr auto read() const noexcept(release) -> std::size_t { return m_read; }

  [[nodiscard]] constexpr auto written() const noexcept(release) -> std::size_t { return m_written; }

private:
  std::size_t m_read;
  std::size_t m_written;
};

} // namespace upd

namespace upd::descriptor {

template<auto Identifier, typename Rule, typename TaggedDescriptions>
struct one_of_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto size = record_size_v<TaggedDescriptions>;
  constexpr static auto alternative_types =
      decltype(tuple_views::concat(
                   typelist2<upd::description<>>,
                   std::declval<TaggedDescriptions>() | record_views::values | tuple_views::to<typelist2_t>)
               | tuple_views::transform_type([]<typename T> -> std::remove_cvref_t<T> {})
               | tuple_views::transform_type([]<typename T> -> typename T::result_type {})
               | tuple_views::to<typelist2_t>){};
  constexpr static auto subinput_types =
      decltype(std::declval<TaggedDescriptions>()
               | record_views::values
               | tuple_views::to<typelist2_t>
               | tuple_views::transform_type([]<typename T> -> std::remove_cvref_t<T> {})
               | tuple_views::transform_type([]<typename T> -> typename T::input_type {})
               | tuple_views::to<typelist2_t>){};

  using input_type = instantiate_variadic<deferred_caster, decltype(subinput_types)>;
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

    auto code = solve_for(code_of<Identifier>, sys | updv::to<std::tuple>);
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
  [[nodiscard]] constexpr auto rules(const Packet &packet, const Fields &fields, Serializer &ser) const {
    namespace updv = upd::record_views;

    auto rule_sys = fields
                    | updv::filter([](auto id, const auto &) { return id != Identifier; })
                    | updv::values
                    | tuple_views::transform([&](const auto &field) { return field.rules(packet, fields, ser); })
                    | tuple_views::join;

    auto sys = tuple_views::concat(std::tuple{code_of<Identifier> = rule}, rule_sys);
    auto id = try_solve_for(code_of<Identifier>, sys);
    auto len = try_solve_for(length_of<Identifier>, sys);
    if constexpr (id == unit || len != unit) {
      return std::tuple{code_of<Identifier> = rule};
    } else {
      auto cnt_stream = counting_stream{};
      encode(get_or<Identifier>(packet, defval), ser, cnt_stream, sys);
      return std::tuple{code_of<Identifier> = rule, length_of<Identifier> = cnt_stream.written() * ser.bytewidth};
    }
  }

  template<serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr auto decode(stream_interface &src, Serializer &ser, const Fields &, const System &sys) const
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
      auto length_rule = tuple_views::fold_left(
          descr.m_fields | record_views::tags | updv::transform([](auto id) { return length_of<id.value>; }),
          0uz,
          [](auto acc, auto var) { return acc + var; });
      auto make_retval = [&](auto &&alt) { return value_type{std::in_place_index<id_pos + 1>, UPD_FWD(alt)}; };
      return descr.decode(src, ser, updv::concat(sys, std::tuple{length_of<Identifier> = length_rule}))
          .transform(make_retval);
    };

    return updv::visit(tagged_descriptions | record_views::values | updv::enumerate, id_pos, make_alt);
  }

  template<typename... Ts>
  [[nodiscard]] constexpr auto bitsize(const std::variant<Ts...> &sum_of_field_values) const noexcept(release)
      -> std::size_t {
    namespace updv = upd::tuple_views;
    auto alt_index = sum_of_field_values.index();
    if (alt_index == 0) {
      return 0;
    }

    auto seq = UPD_WITH_SEQUENCE(Is, sizeof...(Ts) - 1, &) { return std::tuple{expr<Is>...}; };
    return updv::visit(seq, alt_index - 1, [&](auto i) {
      const auto &field_value = *std::get_if<i + 1>(&sum_of_field_values);
      const auto &alt_descr = upd::get_ith<i>(tagged_descriptions);
      return alt_descr.bitsize(field_value);
    });
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr void encode(const input_type &args, Serializer &ser, stream_interface &dest, const System &sys) const
      noexcept(release) {
    namespace updv = upd::tuple_views;

    auto code = solve_for(code_of<Identifier>, sys | updv::to<std::tuple>);
    auto i = updv::dynfind(tags_of_v<TaggedDescriptions>, code);
    auto encode_alt = [&](const auto &named_descr) {
      using target_type = typename std::remove_cvref_t<decltype(named_descr)>::input_type;
      auto target = args.template cast_to<target_type>();

      if constexpr (is_instance_of<decltype(named_descr), description>()) {
        auto length_rule = tuple_views::fold_left(
            named_descr.m_fields | record_views::tags | updv::transform([](auto id) { return length_of<id.value>; }),
            0uz,
            [](auto acc, auto var) { return acc + var; });

        named_descr.encode(target, ser, dest, updv::concat(sys, std::tuple{length_of<Identifier> = length_rule}));
      } else {
        named_descr.encode(target, ser, dest, sys);
      }
    };

    return updv::visit(tagged_descriptions | record_views::values, i, encode_alt);
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
