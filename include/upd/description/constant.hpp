#pragma once

#include <cstddef>
#include <cstdint>
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

template<std::size_t Width>
struct constant_t {
  constexpr static auto width = Width;

  using value_type = unit_t;
  using input_type = unit_t;

  uword_t value;

  template<auto Id, record_like Frame, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Frame &, const Fields &, expr_t<CodecInfo>) const {
    return std::tuple{length_of<Id> = Width};
  }

  template<auto, tuple_like2 System>
  [[nodiscard]] constexpr auto
  encode(input_type, stream_interface &dest, const System &) const
      noexcept(release) -> result<void> {
    if (auto err = dest.write_unsigned(value, width); err) {
      return std::unexpected{found_lite_error{err}};
    }

    return {};
  }

  template<auto, tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, const System &) const -> result<value_type> {
    auto read_value = uword_t{};
    if (auto err = src.read_unsigned(width, &read_value); err) {
      return std::unexpected(found_lite_error{err});
    }

    if (read_value != value) {
      return std::unexpected(
          constant_mismatch{.actual = read_value, .expected = value});
    }

    return {};
  }

  [[nodiscard]] constexpr static auto
  bitsize(value_type) noexcept(release) -> std::size_t {
    return Width;
  }
};

template<std::size_t Width, typename Enum>
struct enumeration_constant_t {
  constexpr static auto width = Width;

  using value_type = unit_t;
  using input_type = unit_t;

  Enum value;

  template<auto Id, record_like Frame, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Frame &, const Fields &, expr_t<CodecInfo>) const {
    return std::tuple{length_of<Id> = Width};
  }

  template<auto, tuple_like2 System>
  [[nodiscard]]
  constexpr auto
  encode(input_type, stream_interface &dest, const System &) const
      -> result<void> {
    if (auto err = dest.write_unsigned(std::to_underlying(value), width); err) {
      return std::unexpected{found_lite_error{err}};
    }

    return {};
  }

  template<auto, tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, const System &) const -> result<value_type> {
    auto read_value = uword_t{};
    if (auto err = src.read_unsigned(width, &read_value); err) {
      return std::unexpected(found_lite_error{err});
    }

    if (static_cast<Enum>(read_value) != value) {
      return std::unexpected(
          constant_mismatch{.actual = read_value,
                            .expected = static_cast<std::uintmax_t>(value)});
    }

    return {};
  }

  [[nodiscard]] constexpr static auto
  bitsize(value_type) noexcept(release) -> std::size_t {
    return Width;
  }
};

template<std::size_t Width>
[[nodiscard]] constexpr auto constant2(uword_t n) noexcept(release) {
  return constant_t<Width>{n};
}

template<std::size_t Width, typename Enum>
  requires std::is_enum_v<Enum>
[[nodiscard]] constexpr auto constant2(Enum e) noexcept(release) {
  return enumeration_constant_t<Width, Enum>{e};
}

} // namespace upd::descriptor
