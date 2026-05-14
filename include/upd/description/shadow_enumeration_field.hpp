#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream_interface.hpp"
#include "../tuple/to.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

template<name Identifier, typename Enum, std::size_t Width>
struct shadow_enumeration_field_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = std::is_signed_v<std::underlying_type_t<Enum>>;
  constexpr static auto width = Width;

  using value_type = Enum;
  using input_type = Enum;

  template<tuple_like2 System, typename... Args>
  [[nodiscard]] constexpr auto make_value(const System &, Args &&...args) const -> value_type {
    return value_type{UPD_FWD(args)...};
  }

  template<tuple_like2 System>
  [[nodiscard]] constexpr static auto default_value(const System &) noexcept(release) -> value_type {
    return value_type{};
  }

  [[nodiscard]] constexpr static auto default_value() noexcept(release) -> value_type { return value_type{}; }

  template<record_like Packet, serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto deduce(Packet &, Serializer &, const Fields &, const System &) noexcept(release) {
    return record{};
  }

  template<serializer Serializer, record_like Packet, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &, Serializer &, const Packet &, const Fields &, const System &sys) -> result<value_type> {
    return solve_for(value_of<Identifier>, sys | tuple_views::to<std::tuple>);
  }

  template<record_like Packet, record_like Fields, serializer Serializer>
  [[nodiscard]] constexpr auto rules(const Packet &packet, const Fields &, Serializer &) const {
    if constexpr (has_tag<Identifier>(packet)) {
      return std::tuple{value_of<Identifier> = get<Identifier>(packet), length_of<Identifier> = Width};
    } else {
      return std::tuple{length_of<Identifier> = Width};
    }
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr static void encode(value_type, Serializer &, stream_interface &, const System &) {}

  [[nodiscard]] constexpr static auto length() noexcept(release) { return Width; }

  template<typename V>
  [[nodiscard]] constexpr auto bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<name Identifier, typename Enum, std::size_t Width>
constexpr auto shadow_efield2 = [] { return description{shadow_enumeration_field_t<Identifier, Enum, Width>{}}; }();

} // namespace upd::descriptor
