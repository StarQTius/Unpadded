#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream_interface.hpp"
#include "../token.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

template<name Identifier, typename Enum, std::size_t Width>
struct enumeration_field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<Enum>>;
  constexpr static auto width = Width;

  using value_type = Enum;

  template<tuple_like2 System, typename... Args>
  [[nodiscard]] constexpr auto make_value(const System &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto default_value(const System &) noexcept(release) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &, const System &) noexcept(release) {
    return record{};
  }

  template<serializer Serializer, record_like Packet, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, Serializer &ser, const Packet &, const Fields &, const System &) -> result<value_type> {
    if constexpr (is_signed) {
      return static_cast<value_type>(ser.deserialize_signed(src, upd::width<width - 1>));
    } else {
      return static_cast<value_type>(ser.deserialize_unsigned(src, upd::width<width>));
    }
  }

  template<record_like Packet, record_like Fields, serializer Serializer>
  [[nodiscard]] constexpr auto rules(const Packet &packet, const Fields &, Serializer &) const {
    if constexpr (has_tag<Identifier>(packet)) {
      return std::tuple{value_of<Identifier> = get<Identifier>(packet), length_of<Identifier> = Width};
    } else {
      return std::tuple{length_of<Identifier> = Width};
    }
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr static void encode(value_type value, Serializer &ser, stream_interface &dest, const System &) {
    if constexpr (is_signed) {
      return ser.serialize_signed(static_cast<std::intmax_t>(value), upd::width<width - 1>, dest);
    } else {
      return ser.serialize_unsigned(static_cast<std::uintmax_t>(value), upd::width<width>, dest);
    }
  }

  [[nodiscard]] constexpr static auto length() noexcept(release) { return Width; }
};

template<typename Enum, std::size_t Width>
struct anonymous_enumeration_field_t {
  using enum_type = Enum;
  constexpr static auto width = Width;

  using result_type = enum_type;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<enum_type>>;

  template<serializer Serializer, record_like Packet, tuple_like2 System>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Packet &, const System &)
      -> result<result_type> {
    auto retval = [&] {
      if constexpr (is_signed) {
        return ser.deserialize_signed(src, upd::width<width - 1>);
      } else {
        return ser.deserialize_unsigned(src, upd::width<width>);
      }
    }();

    return result_type{retval};
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr static void encode(result_type value, Serializer &ser, stream_interface &dest, const System &) {
    auto underlying_value = std::to_underlying(value);

    if constexpr (is_signed) {
      ser.serialize_signed(underlying_value, upd::width<width - 1>, dest);
    } else {
      ser.serialize_unsigned(underlying_value, upd::width<width>, dest);
    }
  }

  [[nodiscard]] constexpr static auto length() noexcept(release) { return Width; }
};

template<name Identifier, typename Enum, std::size_t Width>
constexpr auto efield2 = [] {
  if constexpr (Identifier.anonymous()) {
    return anonymous_enumeration_field_t<Enum, Width>{};
  } else {
    return description{enumeration_field_t<Identifier, Enum, Width>{}};
  }
}();

} // namespace upd::descriptor
