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
#include "codec_info.hpp"

namespace upd::descriptor {

constexpr struct all_fields_t {
} all_fields{};

template<typename BinaryOp, std::size_t Width, typename FieldFilter>
struct checksum_t {
  constexpr static auto width = Width;

  using value_type = unit_t;
  using input_type = unit_t;

  uword_t init;
  BinaryOp op;
  FieldFilter identifier_filter;

  template<auto Id,
           record_like Packet,
           record_like Fields,
           codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &fields, expr_t<CodecInfo>) const {
    using namespace upd::literals;
    namespace updv = upd::record_views;

    auto descr =
        updv::apply([](const auto &...es) { return description{es...}; },
                    fields | updv::filter([]<auto FieldId, typename> {
                      return FieldId != Id;
                    }));

    using descr_input = typename decltype(descr)::input_type;
    auto curated_packet =
        tags_of_v<descr_input>
        | tuple_views::filter([&]<typename Tag> {
            return !std::convertible_to<
                record_element_t<Tag::value, descr_input>, unit_t>;
          })
        | tuple_views::transform([&](auto id) {
            using subinput_type =
                typename record_element_t<id.value, Fields>::input_type;
            return entry{id, get_or<id.value>(packet, subinput_type{})};
          })
        | tuple_views::as_record;

    auto dest = accumulator_stream{&op, init};
    auto res =
        descr.template encode<Id>(curated_packet | updv::to<record>, dest);

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

template<std::size_t Width, typename BinaryOp>
[[nodiscard]] constexpr auto
checksum2(BinaryOp op, all_fields_t) noexcept(release) {
  return checksum_t<BinaryOp, Width, all_fields_t>{0u, std::move(op),
                                                   all_fields};
}

} // namespace upd::descriptor
