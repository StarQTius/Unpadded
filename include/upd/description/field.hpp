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

  template<tuple_like2 System = std::tuple<>>
  constexpr static void encode(value_type value,
                               stream_interface &dest,
                               const System & = std::tuple{}) {
    if constexpr (is_signed) {
      (void)dest.write_signed(value, width);
    } else {
      (void)dest.write_unsigned(value, width);
    }
  }

  template<record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, const Fields &, const System &)
      -> result<value_type> {
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

  template<typename V>
  [[nodiscard]] constexpr auto
  bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<bool Signedness, std::size_t Width>
struct anonymous_field_t {
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using result_type = std::conditional_t<is_signed, word_t, uword_t>;
  using input_type = result_type;

  template<tuple_like2 System = std::tuple<>>
  constexpr void encode(result_type value,
                        stream_interface &dest,
                        const System & = std::tuple{}) const {
    if constexpr (is_signed) {
      (void)dest.write_signed(value, width);
    } else {
      (void)dest.write_unsigned(value, width);
    }
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, const System &) const -> result<result_type> {
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

  template<typename V>
  [[nodiscard]] constexpr auto
  bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<name Identifier, bool Signedness, std::size_t Width>
[[nodiscard]] constexpr auto
field(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
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
