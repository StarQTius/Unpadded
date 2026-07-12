#pragma once

#include <concepts>
#include <cstddef>
#include <expected>
#include <tuple>
#include <utility>

#include "../algebra/system.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../record/tags_of.hpp"
#include "../stream/accumulator_stream.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/as_record.hpp"
#include "../tuple/filter.hpp"
#include "../tuple/transform.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/type_traits.hpp"
#include "codec_info.hpp"

namespace upd::descriptor {

constexpr struct all_fields_t {
} all_fields{};

template<name Identifier,
         typename BinaryOp,
         std::size_t Width,
         typename FieldFilter>
struct checksum_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = uword_t;
  using input_type = unit_t;

  BinaryOp op;
  value_type init;
  FieldFilter identifier_filter;

  template<record_like Packet, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &fields, expr_t<CodecInfo>) const {
    using namespace upd::literals;
    namespace updv = upd::record_views;

    auto descr = updv::apply(
        [](const auto &...es) { return description{es.value...}; },
        fields | updv::filter([](auto id, auto) { return id != Identifier; }));

    using descr_input = typename decltype(descr)::input_type;
    auto curated_packet =
        tags_of_v<descr_input>
        | tuple_views::filter([&]<typename Id>(typebox<const Id &>) {
            return !std::convertible_to<
                record_element_t<Id::value, descr_input>, unit_t>;
          })
        | tuple_views::transform([&](auto id) {
            using subinput_type =
                typename record_element_t<id.value, Fields>::input_type;
            return entry{id, get_or<id.value>(packet, subinput_type{})};
          })
        | tuple_views::as_record;

    auto dest = accumulator_stream{&op, init};
    descr.encode(curated_packet | updv::to<record>, dest);

    return std::tuple{value_of<Identifier> = dest.acc,
                      length_of<Identifier> = Width};
  }

  template<tuple_like2 System>
  constexpr static void
  encode(unit_t, stream_interface &dest, const System &sys) {
    auto value = algebra::solve_for(value_of<Identifier>, sys);
    return (void)dest.write_unsigned(value, width);
  }

  template<record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, const Fields &, const System &sys)
      -> result<value_type> {
    auto actual = uword_t{};
    if (auto err = src.read_unsigned(width, &actual); err) {
      return std::unexpected(found_lite_error{err});
    }

    auto expected = algebra::solve_for(value_of<Identifier>, sys);
    if (actual != expected) {
      return std::unexpected{checksum_mismatch{
          .actual = actual,
          .expected = expected,
      }};
    }

    return actual;
  }

  template<typename V>
  [[nodiscard]] constexpr auto
  bitsize(const V &) const noexcept(release) -> std::size_t {
    return Width;
  }
};

template<name Identifier, std::size_t Width, typename BinaryOp>
[[nodiscard]] constexpr auto
checksum2(BinaryOp op, all_fields_t) noexcept(release) {
  auto is_not_this_field = [](auto id) { return expr<id != Identifier>; };

  auto retval =
      checksum_t<Identifier, BinaryOp, Width, decltype(is_not_this_field)>{
          std::move(op), 0uz, is_not_this_field};

  return description{std::move(retval)};
}

} // namespace upd::descriptor
