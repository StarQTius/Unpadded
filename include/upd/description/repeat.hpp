#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../static_vector.hpp"
#include "../stream_interface.hpp"
#include "../tuple/to.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "serializer.hpp"

namespace upd {

constexpr auto max_repetition = 1024;

} // namespace upd

namespace upd::descriptor {

template<name Identifier, typename Description, typename Rule, std::size_t Max>
struct repeat_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto max = Max;

  using value_type = static_vector<typename Description::result_type, Max>;
  using input_type = value_type;

  template<tuple_like2 System, typename... Args>
  [[nodiscard]] constexpr auto make_value(const System &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }
  using description_type = Description;
  using rule_type = Rule;

  Description description;
  Rule rule;

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto default_value(const System &) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() -> value_type { return value_type{}; }

  template<record_like Packet, record_like Fields, serializer Serializer>
  [[nodiscard]] constexpr auto rules(const Packet &packet, const Fields &fields, Serializer &) const {
    if constexpr (has_tag_v<Identifier, Packet>) {
      return std::tuple{length_of<Identifier> = rule,
                        length_of<Identifier> = get<Identifier>(fields).bitsize(get<Identifier>(packet))};
    } else {
      return std::tuple{length_of<Identifier> = rule};
    }
  }

  template<serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr auto decode(stream_interface &src, Serializer &ser, const Fields &, const System &sys) const
      -> result<value_type> {
    namespace stdv = std::views;
    namespace updv = upd::tuple_views;

    auto len = solve_for(length_of<Identifier>, sys | updv::to<std::tuple>);
    if (len < 0) {
      return std::unexpected{negative_repetition_count{identifier.string, static_cast<std::intmax_t>(len)}};
    }

    auto retval = static_vector<typename Description::result_type, Max>{};

    auto i = 0uz;
    auto overflown = false;
    while (!overflown && i < len) {
      auto maybe_value = description.decode(src, ser, sys);
      if (!maybe_value) {
        return std::unexpected{std::move(maybe_value).error()};
      }
      overflown = !retval.try_push_back(std::move(maybe_value).value());
      i += description.bitsize(retval.back());
    }

    if (overflown) {
      return std::unexpected{repeated_beyond_max{identifier.string, static_cast<std::uintmax_t>(len), Max}};
    }

    return retval;
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr void encode(const value_type &value, Serializer &ser, stream_interface &dest, const System &) const {
    for (const auto &element : value) {
      description.encode(element, ser, dest);
    }
  }

  [[nodiscard]] constexpr static auto length() noexcept(release) { return length_of<Identifier>; }

  template<typename T, std::size_t M>
  [[nodiscard]] constexpr auto bitsize(const static_vector<T, M> &svec) const -> std::size_t {
    namespace stdr = std::ranges;
    return stdr::fold_left(svec, 0uz, [&](auto acc, const auto &elem) { return acc + description.bitsize(elem); });
  }

  template<typename T, std::size_t N>
  [[nodiscard]] constexpr auto bitsize(const std::array<T, N> &arr) const -> std::size_t {
    namespace stdr = std::ranges;
    return stdr::fold_left(arr, 0uz, [&](auto acc, const auto &elem) { return acc + description.bitsize(elem); });
  }
};

template<name Identifier, typename Description, typename Rule>
[[nodiscard]] constexpr auto repeat(Description &&descr, Rule &&rule) {
  using description_type = std::remove_cvref_t<Description>;
  using rule_type = std::remove_cvref_t<Rule>;

  auto retval = repeat_t<Identifier, description_type, rule_type, max_repetition>{
      .description = UPD_FWD(descr),
      .rule = UPD_FWD(rule),
  };

  return description{std::move(retval)};
}

template<name Identifier, typename Description>
[[nodiscard]] constexpr auto repeat(Description &&descr) {
  return repeat<Identifier>(UPD_FWD(descr), length_of<Identifier>);
}

} // namespace upd::descriptor
