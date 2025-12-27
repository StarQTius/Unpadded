#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <ranges>
#include <type_traits>
#include <utility>

#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../static_vector.hpp"
#include "../stream_interface.hpp"
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

  template<record_like Context, typename... Args>
  [[nodiscard]] constexpr auto make_value(const Context &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }
  using description_type = Description;
  using rule_type = Rule;

  Description description;
  Rule rule;

  template<record_like Context>
  [[nodiscard]] constexpr static auto default_value(const Context &) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return record{};
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

} // namespace upd::descriptor
