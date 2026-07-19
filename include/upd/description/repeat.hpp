#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <expected>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../algebra/system.hpp"
#include "../description.hpp"
#include "../error.hpp"
#include "../record.hpp"
#include "../stream/stream_interface.hpp"
#include "../tuple/concat.hpp"
#include "../tuple/to.hpp"
#include "../tuple/tuple_like.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/static_vector.hpp"
#include "codec.hpp"

namespace upd {

constexpr auto max_repetition = 1024;

} // namespace upd

namespace upd::descriptor {

template<codec Description, typename Rule, std::size_t Max>
struct repeat_t {
  constexpr static auto max = Max;

  using value_type = static_vector<typename Description::value_type, Max>;
  using input_type = value_type;
  using description_type = Description;
  using rule_type = Rule;

  Description description;
  Rule rule;

  template<auto Id,
           record_like Packet,
           record_like Fields,
           codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Packet &packet, const Fields &fields, expr_t<CodecInfo>) const {
    auto helper_rule = [&] {
      if constexpr (!std::same_as<unit_t, Rule>) {
        return std::tuple{length_of<Id> = rule};
      } else {
        return std::tuple{};
      }
    }();

    auto bitsize_rule = [&] {
      if constexpr (has_tag_v<Id, Packet>) {
        return std::tuple{length_of<Id> =
                              get<Id>(fields).bitsize(get<Id>(packet))};
      } else {
        return std::tuple{};
      }
    }();

    return tuple_views::concat(helper_rule, bitsize_rule)
           | tuple_views::to<std::tuple>;
  }

  template<auto Id, tuple_like2 System>
  [[nodiscard]]
  constexpr auto encode(const input_type &value,
                        stream_interface &dest,
                        const System &sys) const -> result<void> {
    for (const auto &element : value) {
      if (auto res = description.template encode<Id>(element, dest, sys);
          !res) {
        return res;
      }
    }

    return {};
  }

  template<auto Id, tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src, const System &sys) const -> result<value_type> {
    namespace stdv = std::views;
    namespace updv = upd::tuple_views;

    auto len = solve_for(length_of<Id>, sys | updv::to<std::tuple>);
    if (len < 0) {
      return std::unexpected{
          negative_repetition_count{Id.string, static_cast<word_t>(len)}};
    }

    auto retval = value_type{};

    auto i = 0uz;
    auto overflown = false;
    while (!overflown && i < len) {
      auto maybe_value = description.template decode<Id>(src, sys);
      if (!maybe_value) {
        return std::unexpected{std::move(maybe_value).error()};
      }
      overflown = !retval.try_push_back(std::move(maybe_value).value());
      i += description.bitsize(retval.back());
    }

    if (overflown) {
      return std::unexpected{
          repeated_beyond_max{Id.string, static_cast<uword_t>(len), Max}};
    }

    return retval;
  }

  [[nodiscard]] constexpr auto
  bitsize(const value_type &svec) const -> std::size_t {
    auto retval = 0zu;
    for (const auto &elem : svec) {
      retval += description.bitsize(elem);
    }

    return retval;
  }
};

template<codec Codec>
[[nodiscard]] constexpr auto repeat(Codec &&cdc) {
  return repeat(UPD_FWD(cdc), unit);
}

template<codec Codec, typename Rule>
[[nodiscard]] constexpr auto repeat(Codec &&cdc, Rule &&rule) {
  using codec_type = std::remove_cvref_t<Codec>;
  using rule_type = std::remove_cvref_t<Rule>;

  return repeat_t<codec_type, rule_type, max_repetition>{
      .description = UPD_FWD(cdc),
      .rule = UPD_FWD(rule),
  };
}

template<record_like Codecs, typename Rule>
[[nodiscard]] constexpr auto repeat(Codecs cdcs) {
  return repeat(description{UPD_FWD(cdcs)}, unit);
}

template<record_like Codecs, typename Rule>
[[nodiscard]] constexpr auto repeat(Codecs cdcs, Rule &&rule) {
  return repeat(description{UPD_FWD(cdcs)}, UPD_FWD(rule));
}

} // namespace upd::descriptor
