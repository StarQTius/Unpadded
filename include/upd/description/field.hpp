#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

#include "../constexpr.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream_interface.hpp"
#include "../token.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "codec_info.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

template<name Identifier, bool Signedness, std::size_t Width>
struct field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;
  using input_type = value_type;

  template<tuple_like2 System, typename... Args>
  [[nodiscard]] constexpr auto make_value(const System &, Args &&...args) const -> value_type {
    return value_type(UPD_FWD(args)...);
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto default_value(const System &) noexcept(release) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, record_like Fields, serializer Serializer, codec_info CodecInfo>
  [[nodiscard]] constexpr auto rules(const Packet &packet, const Fields &, Serializer &, expr_t<CodecInfo>) const {
    if constexpr (has_tag<Identifier>(packet)) {
      return std::tuple{value_of<Identifier> = get<Identifier>(packet), length_of<Identifier> = Width};
    } else {
      return std::tuple{length_of<Identifier> = Width};
    }
  }

  template<serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Fields &, const System &)
      -> result<value_type> {
    if constexpr (is_signed) {
      return ser.deserialize_signed(src, upd::width<width>);
    } else {
      return ser.deserialize_unsigned(src, upd::width<width>);
    }
  }

  template<serializer Serializer, tuple_like2 System = std::tuple<>>
  constexpr static void
  encode(value_type value, Serializer &ser, stream_interface &dest, const System & = std::tuple{}) {
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

template<bool Signedness, std::size_t Width>
struct anonymous_field_t {
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using result_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;
  using input_type = result_type;

  template<serializer Serializer, tuple_like2 System = std::tuple<>>
  constexpr void
  encode(result_type value, Serializer &ser, stream_interface &dest, const System & = std::tuple{}) const {
    if constexpr (is_signed) {
      ser.serialize_signed(value, upd::width<width>, dest);
    } else {
      ser.serialize_unsigned(value, upd::width<width>, dest);
    }
  }

  template<serializer Serializer>
  [[nodiscard]] constexpr auto decode(stream_interface &src, Serializer &ser) const -> result<result_type> {
    if constexpr (is_signed) {
      return ser.deserialize_signed(src, upd::width<width>);
    } else {
      return ser.deserialize_unsigned(src, upd::width<width>);
    }
  }

  template<serializer Serializer, tuple_like2 System>
  [[nodiscard]] constexpr auto decode(stream_interface &src, Serializer &ser, const System &) const
      -> result<result_type> {
    return decode(src, ser);
  }

  [[nodiscard]] constexpr static auto length() noexcept(release) { return Width; }

  template<typename V>
  [[nodiscard]] constexpr auto bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<name Identifier, bool Signedness, std::size_t Width>
[[nodiscard]] constexpr auto field(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
  field_like auto retval = field_t<Identifier, Signedness, Width>{};

  return description{retval};
}

template<name Identifier, std::size_t Width>
constexpr auto field2 = [] {
  if constexpr (Identifier.anonymous()) {
    return anonymous_field_t<true, Width>{};
  } else {
    return description{field_t<Identifier, true, Width>{}};
  }
}();

template<name Identifier, std::size_t Width>
constexpr auto ufield2 = [] {
  if constexpr (Identifier.anonymous()) {
    return anonymous_field_t<false, Width>{};
  } else {
    return description{field_t<Identifier, false, Width>{}};
  }
}();

} // namespace upd::descriptor
