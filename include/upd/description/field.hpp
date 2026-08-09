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

template<bool Signedness, std::size_t Width>
struct field_t {
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, word_t, uword_t>;
  using input_type = value_type;

  template<auto Id, record_like Frame, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr static auto
  rules(const Frame &frm, const Fields &, expr_t<CodecInfo>) {
    if constexpr (has_tag<Id>(frm)) {
      return std::tuple{value_of<Id> = get<Id>(frm), length_of<Id> = Width};
    } else {
      return std::tuple{length_of<Id> = Width};
    }
  }

  template<auto, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  encode(input_type value, stream_interface &dest, const System &)
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

  template<auto, tuple_like2 System>
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

  [[nodiscard]] constexpr static auto
  bitsize(value_type) noexcept(release) -> std::size_t {
    return Width;
  }
};

template<std::size_t Width>
constexpr auto field2 = field_t<true, Width>{};

template<std::size_t Width>
constexpr auto ufield2 = field_t<false, Width>{};

} // namespace upd::descriptor
