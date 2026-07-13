#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/to.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "codec_info.hpp"

namespace upd::descriptor {

template<name Identifier, typename Enum, std::size_t Width>
struct shadow_enumeration_field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed =
      std::is_signed_v<std::underlying_type_t<Enum>>;
  constexpr static auto width = Width;

  using value_type = Enum;
  using input_type = Enum;

  template<record_like Packet, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &, expr_t<CodecInfo>) const {
    if constexpr (has_tag<Identifier>(packet)
                  && CodecInfo.operation
                  == codec_operation::encoding) {
      return std::tuple{value_of<Identifier> = get<Identifier>(packet),
                        length_of<Identifier> = Width};
    } else {
      return std::tuple{length_of<Identifier> = Width};
    }
  }

  template<tuple_like2 System>
  [[nodiscard]]
  constexpr static auto
  encode(value_type, stream_interface &, const System &) -> result<void> {
    return result<void>{};
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &, const System &sys) -> result<value_type> {
    return solve_for(value_of<Identifier>, sys | tuple_views::to<std::tuple>);
  }

  [[nodiscard]] constexpr auto
  bitsize(value_type) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<name Identifier, typename Enum, std::size_t Width>
constexpr auto shadow_efield2 = [] {
  return description{shadow_enumeration_field_t<Identifier, Enum, Width>{}};
}();

} // namespace upd::descriptor
