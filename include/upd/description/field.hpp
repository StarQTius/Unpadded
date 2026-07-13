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

template<name Identifier, bool Signedness, std::size_t Width>
struct field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, word_t, uword_t>;
  using input_type = value_type;

  template<record_like Packet, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &, expr_t<CodecInfo>) const {
    if constexpr (has_tag<Identifier>(packet)) {
      return std::tuple{value_of<Identifier> = get<Identifier>(packet),
                        length_of<Identifier> = Width};
    } else {
      return std::tuple{length_of<Identifier> = Width};
    }
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto
  encode(value_type value, stream_interface &dest, const System &)
      -> result<void> {
    auto err = lite_error::none;
    if constexpr (is_signed) {
      err = dest.write_signed(value, width);
    } else {
      err = dest.write_unsigned(value, width);
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
    auto retval = uword_t{};
    if constexpr (is_signed) {
      err = src.read_signed(width, &retval);
    } else {
      err = src.read_unsigned(width, &retval);
    }

    if (err) {
      return std::unexpected(found_lite_error{err});
    }

    return retval;
  }

  [[nodiscard]] constexpr auto
  bitsize(value_type) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<bool Signedness, std::size_t Width>
struct anonymous_field_t {
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, word_t, uword_t>;
  using input_type = value_type;

  template<record_like Packet, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &, const Fields &, expr_t<CodecInfo>) const {
    return std::tuple{};
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto
  encode(value_type value, stream_interface &dest, const System &) const
      -> result<void> {
    auto err = lite_error::none;
    if constexpr (is_signed) {
      err = dest.write_signed(value, width);
    } else {
      err = dest.write_unsigned(value, width);
    }

    if (err) {
      return std::unexpected(found_lite_error{err});
    }

    return {};
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, const System &) const -> result<value_type> {
    auto err = lite_error::none;
    auto retval = uword_t{};
    if constexpr (is_signed) {
      err = src.read_signed(width, &retval);
    } else {
      err = src.read_unsigned(width, &retval);
    }

    if (err) {
      return std::unexpected(found_lite_error{err});
    }

    return retval;
  }

  [[nodiscard]] constexpr auto
  bitsize(value_type) const noexcept(release) -> std::size_t {
    return Width;
  }
};

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
