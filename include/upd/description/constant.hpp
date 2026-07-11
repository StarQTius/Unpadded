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
#include "../stream/stream_interface.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/token.hpp"
#include "codec_info.hpp"

namespace upd::descriptor {

template<name Identifier, std::size_t Width>
struct constant_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = uword_t;
  using input_type = unit_t;

  value_type field_value;

  template<record_like Packet, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &, const Fields &, expr_t<CodecInfo>) const {
    return std::tuple{length_of<Identifier> = Width};
  }

  template<tuple_like2 System>
  constexpr void encode(unit_t, stream_interface &dest, const System &) const
      noexcept(release) {
    (void)dest.write_unsigned(field_value, width);
  }

  template<record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, const Fields &, const System &)
      -> result<value_type> {
    auto retval = value_type{};
    if (auto err = src.read_unsigned(width, &retval); err) {
      return std::unexpected(found_lite_error{err});
    }

    return retval;
  }

  template<typename V>
  [[nodiscard]] constexpr auto
  bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<name Identifier, std::size_t Width, typename Enum>
struct enumeration_constant_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = Enum;
  using input_type = unit_t;

  template<record_like Packet, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &, const Fields &, expr_t<CodecInfo>) const {
    return std::tuple{length_of<Identifier> = Width};
  }

  template<tuple_like2 System>
  constexpr void encode(unit_t, stream_interface &dest, const System &) const {
    (void)dest.write_unsigned(std::to_underlying(field_value), width);
  }

  template<record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, const Fields &, const System &)
      -> result<value_type> {
    auto retval = uword_t{};
    if (auto err = src.read_unsigned(width, &retval); err) {
      return std::unexpected(found_lite_error{err});
    }

    return static_cast<Enum>(retval);
  }

  template<typename V>
  [[nodiscard]] constexpr auto
  bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }

  Enum field_value;
};

template<name Identifier, typename T, std::size_t Width>
[[nodiscard]] constexpr auto constant(T n, width_t<Width>) noexcept(release) {
  auto retval = constant_t<Identifier, Width>{n};
  return description{retval};
}

template<name Identifier, std::size_t Width>
[[nodiscard]] constexpr auto constant2(uword_t n) noexcept(release) {
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
