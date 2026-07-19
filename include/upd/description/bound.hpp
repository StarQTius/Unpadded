#pragma once

#include <cstddef>
#include <expected>
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
#include "codec_info.hpp"

namespace upd::descriptor {

template<bool Signedness, std::size_t Width, typename Rule>
struct bound_t {
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, word_t, uword_t>;
  using input_type = unit_t;
  using rule_type = Rule;

  Rule rule;

  template<auto Id,
           record_like Packet,
           record_like Fields,
           codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &, expr_t<CodecInfo>) const {
    if constexpr (CodecInfo.operation
                  == codec_operation::decoding
                  && has_tag<Id>(packet)) {
      return std::tuple{value_of<Id> = get<Id>(packet), value_of<Id> = rule,
                        length_of<Id> = Width};
    } else {
      return std::tuple{value_of<Id> = rule, length_of<Id> = Width};
    }
  };

  template<auto Id, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  encode(unit_t, stream_interface &dest, const System &sys) -> result<void> {
    auto err = lite_error::none;
    auto value = algebra::solve_for(value_of<Id>, sys);
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

  template<auto, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, const System &) -> result<value_type> {
    auto err = lite_error_t{};
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

template<std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto bound2(Rule rule) noexcept(release) {
  return bound_t<true, Width, Rule>{std::move(rule)};
}

template<std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto ubound2(Rule rule) noexcept(release) {
  return bound_t<false, Width, Rule>{std::move(rule)};
}

} // namespace upd::descriptor
