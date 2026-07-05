#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/token.hpp"
#include "codec_info.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

template<name Identifier, typename Enum, std::size_t Width>
struct enumeration_field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<Enum>>;
  constexpr static auto width = Width;

  using value_type = Enum;
  using input_type = Enum;

  template<record_like Packet, record_like Fields, serializer Serializer, codec_info CodecInfo>
  [[nodiscard]] constexpr auto rules(const Packet &packet, const Fields &, Serializer &, expr_t<CodecInfo>) const {
    if constexpr (has_tag<Identifier>(packet)) {
      return std::tuple{value_of<Identifier> = get<Identifier>(packet), length_of<Identifier> = Width};
    } else {
      return std::tuple{length_of<Identifier> = Width};
    }
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr static void encode(Enum value, Serializer &ser, stream_interface &dest, const System &) {
    if constexpr (is_signed) {
      return ser.serialize_signed(static_cast<std::intmax_t>(value), upd::width<width - 1>, dest);
    } else {
      return ser.serialize_unsigned(static_cast<std::uintmax_t>(value), upd::width<width>, dest);
    }
  }

  template<serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const Fields &, const System &)
      -> result<value_type> {
    if constexpr (is_signed) {
      return static_cast<value_type>(ser.deserialize_signed(src, upd::width<width - 1>));
    } else {
      return static_cast<value_type>(ser.deserialize_unsigned(src, upd::width<width>));
    }
  }

  template<typename V>
  [[nodiscard]] constexpr auto bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<typename Enum, std::size_t Width>
struct anonymous_enumeration_field_t {
  using enum_type = Enum;
  constexpr static auto width = Width;

  using result_type = enum_type;
  using input_type = enum_type;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<enum_type>>;

  template<record_like Fields, serializer Serializer, codec_info CodecInfo>
  [[nodiscard]] constexpr auto rules(Enum, const Fields &, Serializer &, expr_t<CodecInfo>) const {
    return std::tuple{};
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr static void encode(Enum value, Serializer &ser, stream_interface &dest, const System &) {
    if constexpr (is_signed) {
      ser.serialize_signed(static_cast<std::intmax_t>(value), upd::width<width - 1>, dest);
    } else {
      ser.serialize_unsigned(static_cast<std::uintmax_t>(value), upd::width<width>, dest);
    }
  }

  template<serializer Serializer, tuple_like2 System>
  [[nodiscard]] constexpr static auto decode(stream_interface &src, Serializer &ser, const System &)
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

  template<typename V>
  [[nodiscard]] constexpr auto bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }
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
