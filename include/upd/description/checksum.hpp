#pragma once

#include <cstddef>
#include <expected>
#include <tuple>
#include <utility>

#include "../algebra/system.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream/accumulator_stream.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/find.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "codec_info.hpp"

namespace upd::descriptor {

template<typename BinaryOp, std::size_t Width, typename KeyPred>
struct checksum_t {
  constexpr static auto width = Width;

  using value_type = unit_t;
  using input_type = unit_t;

  uword_t init;
  BinaryOp op;
  KeyPred key_pred;

  template<auto Id,
           record_like Packet,
           record_like Fields,
           codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &fields, expr_t<CodecInfo>) const {
    using namespace upd::literals;
    using namespace upd::record_views;

    auto p = [key_pred = key_pred]<auto K, typename> {
      return UPD_INVOKE_TEMPLATE(key_pred, (K));
    };
    auto curated_fields = fields | take_until<Id> | filter(p);
    auto curated_packet = packet | take_until<Id> | filter(p);

    auto descr = description{curated_fields | to<record>};
    auto dest = accumulator_stream{&op, init};
    auto res = descr.template encode<Id>(curated_packet, dest);
    return std::tuple{value_of<Id> = (res) ? dest.acc : 0zu,
                      length_of<Id> = Width};
  }

  template<auto Id, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  encode(unit_t, stream_interface &dest, const System &sys) -> result<void> {
    auto value = algebra::solve_for(value_of<Id>, sys);
    if (auto err = dest.write_unsigned(value, width); err) {
      return std::unexpected{found_lite_error{err}};
    }

    return {};
  }

  template<auto Id, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, const System &sys) -> result<value_type> {
    auto actual = uword_t{};
    if (auto err = src.read_unsigned(width, &actual); err) {
      return std::unexpected(found_lite_error{err});
    }

    auto expected = algebra::solve_for(value_of<Id>, sys);
    if (actual != expected) {
      return std::unexpected{checksum_mismatch{
          .actual = actual,
          .expected = expected,
      }};
    }

    return {};
  }

  [[nodiscard]] constexpr auto
  bitsize(unit_t) const noexcept(release) -> std::size_t {
    return Width;
  }
};

constexpr struct all_previous_fields_t {
} all_previous_fields;

template<std::size_t Width, typename BinaryOp>
[[nodiscard]] constexpr auto
checksum2(BinaryOp op, all_previous_fields_t) noexcept(release) {
  auto pred = []<auto> { return true; };
  return checksum_t<BinaryOp, Width, decltype(pred)>{0u, std::move(op), pred};
}

template<auto... Identifiers>
struct only_fields_t {};

template<name... Identifiers>
constexpr auto only_fields = only_fields_t<Identifiers...>{};

template<std::size_t Width, typename BinaryOp, auto... Identifiers>
[[nodiscard]] constexpr auto
checksum2(BinaryOp op, only_fields_t<Identifiers...>) noexcept(release) {
  using namespace upd::tuple_views;

  auto pred = []<auto Id> {
    auto ids = std::tuple{expr<Identifiers>...};
    return find<expr_t<Id>>(ids) != sizeof...(Identifiers);
  };
  return checksum_t<BinaryOp, Width, decltype(pred)>{0u, std::move(op), pred};
}

} // namespace upd::descriptor
