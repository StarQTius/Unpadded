#pragma once

#include <cstddef>
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

template<typename Enum, std::size_t Width>
struct shadow_enumeration_field_t {
  constexpr static auto is_signed =
      std::is_signed_v<std::underlying_type_t<Enum>>;
  constexpr static auto width = Width;

  using value_type = Enum;
  using input_type = Enum;

  template<auto Id,
           record_like Packet,
           record_like Fields,
           codec_info CodecInfo>
  [[nodiscard]] constexpr static auto
  rules(const Packet &packet, const Fields &, expr_t<CodecInfo>) {
    if constexpr (has_tag<Id>(packet)
                  && CodecInfo.operation
                  == codec_operation::encoding) {
      return std::tuple{value_of<Id> = get<Id>(packet), length_of<Id> = Width};
    } else {
      return std::tuple{length_of<Id> = Width};
    }
  }

  template<auto, tuple_like2 System>
  [[nodiscard]]
  constexpr static auto
  encode(value_type, stream_interface &, const System &) noexcept(release)
      -> result<void> {
    return result<void>{};
  }

  template<auto Id, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &, const System &sys) -> result<value_type> {
    return solve_for(value_of<Id>, sys);
  }

  [[nodiscard]] constexpr static auto
  bitsize(value_type) noexcept(release) -> std::size_t {
    return Width;
  }
};

template<typename Enum, std::size_t Width>
constexpr auto shadow_efield2 = shadow_enumeration_field_t<Enum, Width>{};

} // namespace upd::descriptor
