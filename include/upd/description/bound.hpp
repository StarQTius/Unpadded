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
#include "../tuple/to.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/token.hpp"
#include "codec_info.hpp"

namespace upd::descriptor {

template<name Identifier, bool Signedness, std::size_t Width, typename Rule>
struct bound_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;

  using value_type = std::conditional_t<is_signed, word_t, uword_t>;
  using input_type = unit_t;
  using rule_type = Rule;

  Rule rule;

  template<record_like Packet, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &, expr_t<CodecInfo>) const {
    if constexpr (CodecInfo.operation
                  == codec_operation::decoding
                  && has_tag<Identifier>(packet)) {
      return std::tuple{value_of<Identifier> = get<Identifier>(packet),
                        value_of<Identifier> = rule,
                        length_of<Identifier> = Width};
    } else {
      return std::tuple{value_of<Identifier> = rule,
                        length_of<Identifier> = Width};
    }
  };

  template<tuple_like2 System>
  constexpr static void
  encode(unit_t, stream_interface &dest, const System &sys) {
    auto value = algebra::solve_for(value_of<Identifier>,
                                    sys | tuple_views::to<std::tuple>);
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

  template<typename V>
  [[nodiscard]] constexpr auto
  bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<name Identifier, bool Signedness, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto
bound(signedness_t<Signedness>, width_t<Width>, Rule rule) noexcept(release) {
  auto retval = bound_t<Identifier, Signedness, Width, Rule>{std::move(rule)};

  return description{std::move(retval)};
}

template<name Identifier, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto bound2(Rule rule) noexcept(release) {
  auto retval = bound_t<Identifier, true, Width, Rule>{std::move(rule)};

  return description{std::move(retval)};
}

template<name Identifier, std::size_t Width, typename Rule>
[[nodiscard]] constexpr auto ubound2(Rule rule) noexcept(release) {
  auto retval = bound_t<Identifier, false, Width, Rule>{std::move(rule)};

  return description{std::move(retval)};
}

} // namespace upd::descriptor
