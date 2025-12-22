#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream_interface.hpp"
#include "../token.hpp"
#include "../upd.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

template<name Identifier, bool Signedness, std::size_t Width>
struct field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;

  template<record_like Context, typename... Args>
  [[nodiscard]] constexpr auto make_value(const Context &, Args &&...args) const -> value_type {
    return value_type(UPD_FWD(args)...);
  }

  template<record_like Context>
  [[nodiscard]] constexpr static auto default_value(const Context &) noexcept(release) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &) noexcept(release) {
    return record{};
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

template<bool Signedness, std::size_t Width>
struct anonymous_field_t {
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using result_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;

  template<serializer Serializer>
  constexpr void encode(result_type value, Serializer &ser, stream_interface &dest) const {
    if constexpr (is_signed) {
      ser.serialize_signed(value, upd::width<width>, dest);
    } else {
      ser.serialize_unsigned(value, upd::width<width>, dest);
    }
  }

  template<serializer Serializer, typename Packet>
  [[nodiscard]] constexpr auto decode(stream_interface &src, Serializer &ser, const Packet &) const
      -> result<result_type> {
    if constexpr (is_signed) {
      return ser.deserialize_signed(src, upd::width<width>);
    } else {
      return ser.deserialize_unsigned(src, upd::width<width>);
    }
  }
};

template<name Identifier, bool Signedness, std::size_t Width>
[[nodiscard]] constexpr auto field(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
  field_like auto retval = field_t<Identifier, Signedness, Width>{};

  return description{retval};
}

template<name Identifier, std::size_t Width>
constexpr auto field2 = [] {
  if constexpr (Identifier.anonymous) {
    return anonymous_field_t<true, Width>{};
  } else {
    return description{field_t<Identifier, true, Width>{}};
  }
}();

template<name Identifier, std::size_t Width>
constexpr auto ufield2 = [] {
  if constexpr (Identifier.anonymous) {
    return anonymous_field_t<false, Width>{};
  } else {
    return description{field_t<Identifier, false, Width>{}};
  }
}();

} // namespace upd::descriptor
