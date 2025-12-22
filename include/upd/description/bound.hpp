#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "../constexpr.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream_interface.hpp"
#include "../token.hpp"
#include "../upd.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

template<name Identifier, bool Signedness, std::size_t Width, typename Rule>
struct bound_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;

  template<record_like Context, typename... Args>
  [[nodiscard]] constexpr auto make_value(const Context &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }
  using rule_type = Rule;

  rule_type rule;

  template<record_like Context>
  [[nodiscard]] constexpr static auto default_value(const Context &) noexcept(release) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr auto deduce(Packet &packet, Serializer &, const Fields &fields) const {
    return record{entry{expr<identifier>, rule.deduce(std::as_const(packet), fields)}};
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

template<name Identifier, bool Signedness, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto bound(signedness_t<Signedness>, width_t<Width>, Rule rule) noexcept(release) {
  auto retval = bound_t<Identifier, Signedness, Width, Rule>{std::move(rule)};

  return description{std::move(retval)};
}

template<name Identifier, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto bound2(Rule rule) noexcept(release) {
  auto retval = bound_t<Identifier, true, Width, Rule>{std::move(rule)};

  return description{std::move(retval)};
}

template<name Identifier, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto ubound2(Rule rule) noexcept(release) {
  auto retval = bound_t<Identifier, false, Width, Rule>{std::move(rule)};

  return description{std::move(retval)};
}

} // namespace upd::descriptor
