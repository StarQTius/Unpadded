#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../algebra/system.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream_interface.hpp"
#include "../token.hpp"
#include "../tuple/to.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

template<name Identifier, bool Signedness, std::size_t Width, typename Rule>
struct bound_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;
  using input_type = unit_t;

  template<tuple_like2 System, typename... Args>
  [[nodiscard]] constexpr auto make_value(const System &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }
  using rule_type = Rule;

  rule_type rule;

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto default_value(const System &) noexcept(release) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, record_like Fields, serializer Serializer>
  [[nodiscard]] constexpr auto rules(const Packet &packet, const Fields &, Serializer &) const {
    if constexpr (has_tag<Identifier>(packet)) {
      return std::tuple{
          value_of<Identifier> = get<Identifier>(packet), value_of<Identifier> = rule, length_of<Identifier> = Width};
    } else {
      return std::tuple{value_of<Identifier> = rule, length_of<Identifier> = Width};
    }
  };

  template<serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Fields &, const System &)
      -> result<value_type> {
    if constexpr (is_signed) {
      return ser.deserialize_signed(src, upd::width<width>);
    } else {
      return ser.deserialize_unsigned(src, upd::width<width>);
    }
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr static void encode(unit_t, Serializer &ser, stream_interface &dest, const System &sys) {
    auto value = algebra::solve_for(value_of<Identifier>, sys | tuple_views::to<std::tuple>);
    if constexpr (is_signed) {
      return ser.serialize_signed(value, upd::width<width>, dest);
    } else {
      return ser.serialize_unsigned(value, upd::width<width>, dest);
    }
  }

  [[nodiscard]] constexpr static auto length() noexcept(release) { return Width; }

  template<typename V>
  [[nodiscard]] constexpr auto bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
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
