#pragma once

#include <cstddef>
#include <expected>
#include <tuple>
#include <type_traits>

#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "codec_info.hpp"

namespace upd::descriptor {

template<name Identifier, typename Enum, std::size_t Width>
struct enumeration_field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed =
      std::is_signed_v<std::underlying_type_t<Enum>>;
  constexpr static auto width = Width;

  using value_type = Enum;
  using input_type = Enum;

  template<record_like Packet, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr static auto
  rules(const Packet &packet,
        const Fields &,
        expr_t<CodecInfo>) noexcept(release) {
    if constexpr (has_tag<Identifier>(packet)) {
      return std::tuple{value_of<Identifier> = get<Identifier>(packet),
                        length_of<Identifier> = Width};
    } else {
      return std::tuple{length_of<Identifier> = Width};
    }
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto
  encode(input_type value, stream_interface &dest, const System &)
      -> result<void> {
    auto err = lite_error::none;
    if constexpr (is_signed) {
      err = dest.write_signed(static_cast<word_t>(value), width);
    } else {
      err = dest.write_unsigned(static_cast<uword_t>(value), width);
    }

    if (err) {
      return std::unexpected{found_lite_error{err}};
    }

    return {};
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, const System &) -> result<value_type> {
    auto err = lite_error::none;
    auto raw = uword_t{};
    if constexpr (is_signed) {
      err = src.read_signed(width, &raw);
    } else {
      err = src.read_unsigned(width, &raw);
    }

    auto retval = static_cast<value_type>(raw);
    if (err) {
      return std::unexpected(found_lite_error{err});
    }

    return retval;
  }

  [[nodiscard]] constexpr static auto
  bitsize(value_type) noexcept(release) -> std::size_t {
    return Width;
  }
};

template<typename Enum, std::size_t Width>
struct anonymous_enumeration_field_t {
  constexpr static auto width = Width;
  constexpr static auto is_signed =
      std::is_signed_v<std::underlying_type_t<Enum>>;

  using value_type = Enum;
  using input_type = Enum;

  template<record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr static auto
  rules(Enum, const Fields &, expr_t<CodecInfo>) noexcept(release) {
    return std::tuple{};
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto
  encode(input_type value, stream_interface &dest, const System &)
      -> result<void> {
    auto err = lite_error::none;
    if constexpr (is_signed) {
      err = dest.write_signed(static_cast<word_t>(value), width);
    } else {
      err = dest.write_unsigned(static_cast<uword_t>(value), width);
    }

    if (err) {
      return std::unexpected{found_lite_error{err}};
    }

    return {};
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, const System &) -> result<value_type> {
    auto err = lite_error::none;
    auto raw = uword_t{};
    if constexpr (is_signed) {
      err = src.read_signed(width, &raw);
    } else {
      err = src.read_unsigned(width, &raw);
    }

    auto retval = static_cast<value_type>(raw);
    if (err) {
      return std::unexpected(found_lite_error{err});
    }

    return retval;
  }

  [[nodiscard]] constexpr static auto
  bitsize(value_type) noexcept(release) -> std::size_t {
    return Width;
  }
};

template<name Identifier, typename Enum, std::size_t Width>
constexpr auto efield2 = [] {
  if constexpr (Identifier.anonymous()) {
    return anonymous_enumeration_field_t<Enum, Width>{};
  } else {
    return enumeration_field_t<Identifier, Enum, Width>{};
  }
}();

} // namespace upd::descriptor
