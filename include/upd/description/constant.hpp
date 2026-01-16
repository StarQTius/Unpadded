#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream_interface.hpp"
#include "../token.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

template<name Identifier, std::size_t Width>
struct constant_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = std::uintmax_t;

  template<tuple_like2 System, typename... Args>
  [[nodiscard]] constexpr auto make_value(const System &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  value_type field_value;

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto default_value(const System &) const noexcept(release) -> value_type {
    return field_value;
  }

  [[nodiscard]] constexpr auto default_value() const noexcept(release) -> value_type { return field_value; }

  template<record_like Packet, serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &, const System &) noexcept(release) {
    return record{};
  }

  template<record_like Packet, record_like Fields, serializer Serializer>
  [[nodiscard]] constexpr auto rules(const Packet &, const Fields &, Serializer &) const {
    return std::tuple{};
  }

  template<serializer Serializer, record_like Packet, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, Serializer &ser, const Packet &, const Fields &, const System &) -> result<value_type> {
    return ser.deserialize_unsigned(src, upd::width<width>);
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr void encode(value_type, Serializer &ser, stream_interface &dest, const System &) const noexcept(release) {
    return ser.serialize_unsigned(field_value, upd::width<width>, dest);
  }

  [[nodiscard]] constexpr static auto length() noexcept(release) { return Width; }
};

template<name Identifier, std::size_t Width, typename Enum>
struct enumeration_constant_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = Enum;

  template<tuple_like2 System, typename... Args>
  [[nodiscard]] constexpr auto make_value(const System &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto default_value(const System &) const noexcept(release) -> value_type {
    return field_value;
  }

  [[nodiscard]] constexpr auto default_value() const noexcept(release) -> value_type { return field_value; }

  template<record_like Packet, serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &, const System &) noexcept(release) {
    return record{};
  }

  template<record_like Packet, record_like Fields, serializer Serializer>
  [[nodiscard]] constexpr auto rules(const Packet &, const Fields &, Serializer &) const {
    return std::tuple{};
  }

  template<serializer Serializer, record_like Packet, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, Serializer &ser, const Packet &, const Fields &, const System &) -> result<value_type> {
    return static_cast<Enum>(ser.deserialize_unsigned(src, upd::width<width>));
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr static void encode(value_type value, Serializer &ser, stream_interface &dest, const System &) {
    return ser.serialize_unsigned(static_cast<std::uintmax_t>(value), upd::width<width>, dest);
  }

  [[nodiscard]] constexpr static auto length() noexcept(release) { return Width; }

  Enum field_value;
};

template<name Identifier, typename T, std::size_t Width>
[[nodiscard]] constexpr auto constant(T n, width_t<Width>) noexcept(release) {
  auto retval = constant_t<Identifier, Width>{n};
  return description{retval};
}

template<name Identifier, std::size_t Width>
[[nodiscard]] constexpr auto constant2(std::uintmax_t n) noexcept(release) {
  auto retval = constant_t<Identifier, Width>{n};
  return description{retval};
}

template<name Identifier, std::size_t Width, typename Enum>
  requires std::is_enum_v<Enum>
[[nodiscard]] constexpr auto constant2(Enum e) noexcept(release) {
  auto retval = enumeration_constant_t<Identifier, Width, Enum>{e};
  return description{retval};
}

} // namespace upd::descriptor
