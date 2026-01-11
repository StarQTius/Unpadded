#pragma once

#include <cstddef>
#include <cstdint>
#include <ranges>
#include <tuple>
#include <utility>

#include "../constexpr.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream_interface.hpp"
#include "../token.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "serializer.hpp"

namespace upd::descriptor {

constexpr struct all_fields_t {
} all_fields{};

template<name Identifier, typename BinaryOp, std::size_t Width, typename FieldFilter>
struct checksum_t {
  constexpr static auto identifier = Identifier;
  constexpr static auto width = Width;

  using value_type = std::uintmax_t;

  template<tuple_like2 System, typename... Args>
  [[nodiscard]] constexpr auto make_value(const System &, Args &&...args) const -> value_type {
    return value_type(UPD_FWD(args)...);
  }

  BinaryOp op;
  value_type init;
  FieldFilter identifier_filter;

  template<tuple_like2 System>
  [[nodiscard]] constexpr auto default_value(const System &) const noexcept(release) -> value_type {
    return init;
  }

  [[nodiscard]] constexpr auto default_value() const noexcept(release) -> value_type { return init; }

  template<record_like Packet, serializer Serializer, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr auto deduce(Packet &packet, Serializer &ser, const Fields &fields, const System &sys) const {
    using namespace upd::literals;
    namespace updv = upd::record_views;

    struct stream_t : stream_interface {
      stream_t(const BinaryOp *op, value_type acc) : op{op}, acc{acc} {}

      auto read(std::size_t, word_t *) -> stream_error_t override { return 1; }

      auto write(const word_t *src, std::size_t size) -> stream_error_t override {
        namespace stdr = std::ranges;

        for (auto w : stdr::subrange{src, src + size}) {
          acc = UPD_INVOKE(*op, acc, static_cast<value_type>(w));
        }

        return 0;
      }

      const BinaryOp *op;
      value_type acc;
    } dest{&op, init};

    auto field_filter = [&]<auto Id>(expr_t<Id>, auto) { return UPD_INVOKE(FieldFilter{}, expr<Id>); };

    updv::for_each(updv::zip(packet | updv::filter(field_filter), fields), [&](auto, const auto &value_and_field) {
      const auto &[value, field] = value_and_field;
      field.encode(value, ser, dest, sys);
    });

    return record{entry{expr<identifier>, dest.acc}};
  }

  template<record_like Packet, record_like Fields>
  [[nodiscard]] constexpr auto rules(const Packet &, const Fields &) const {
    return std::tuple{};
  }

  template<serializer Serializer, record_like Packet, record_like Fields, tuple_like2 System>
  [[nodiscard]] constexpr static auto
  decode(stream_interface &src, Serializer &ser, const Packet &, const Fields &, const System &) -> result<value_type> {
    return ser.deserialize_unsigned(src, upd::width<width>);
  }

  template<serializer Serializer, tuple_like2 System>
  constexpr static void encode(value_type value, Serializer &ser, stream_interface &dest, const System &) {
    return ser.serialize_unsigned(value, upd::width<width>, dest);
  }

  [[nodiscard]] constexpr auto length() const noexcept(release) { return Width; }
};

template<name Identifier, typename BinaryOp, std::size_t Width>
[[nodiscard]] constexpr auto
checksum(BinaryOp op, std::uintmax_t init, width_t<Width>, all_fields_t) noexcept(release) {
  auto is_not_this_field = [](auto id) { return expr<id != Identifier>; };

  auto retval =
      checksum_t<Identifier, BinaryOp, Width, decltype(is_not_this_field)>{std::move(op), init, is_not_this_field};

  return description{std::move(retval)};
}

template<name Identifier, std::size_t Width, typename BinaryOp>
[[nodiscard]] constexpr auto checksum2(BinaryOp op, all_fields_t) noexcept(release) {
  auto is_not_this_field = [](auto id) { return expr<id != Identifier>; };

  auto retval =
      checksum_t<Identifier, BinaryOp, Width, decltype(is_not_this_field)>{std::move(op), 0uz, is_not_this_field};

  return description{std::move(retval)};
}

} // namespace upd::descriptor
