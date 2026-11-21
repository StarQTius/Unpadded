#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <expected>
#include <memory_resource>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

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
#include "../utility/deferred_caster.hpp"
#include "codec.hpp"
#include "frame_segment.hpp"

namespace upd::descriptor {

template<codec Description, typename Rule>
struct repeat_t {
  using value_type = frame_segment<typename Description::value_type>;
  using input_type = deferred_range_caster<typename Description::input_type>;
  using description_type = Description;
  using rule_type = Rule;

  Description description;
  Rule rule;

  template<auto Id, record_like Frame, record_like Fields, codec_info CodecInfo>
  [[nodiscard]] constexpr auto
  rules(const Frame &frm, const Fields &fields, expr_t<CodecInfo>) const {
    auto helper_rule = [&] {
      if constexpr (!std::same_as<unit_t, Rule>) {
        return std::tuple{length_of<Id> = rule};
      } else {
        return std::tuple{};
      }
    }();

    auto bitsize_rule = [&] {
      if constexpr (has_tag_v<Id, Frame>) {
        return std::tuple{length_of<Id> =
                              get<Id>(fields).bitsize(get<Id>(frm))};
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
    auto rg = value.template cast_to<typename description_type::input_type>();
    for (const auto &element : rg) {
      if (auto res = description.template encode<Id>(element, dest, sys);
          !res) {
        return res;
      }
    }

    return {};
  }

  template<auto Id, tuple_like2 System>
  [[nodiscard]] constexpr auto
  decode(stream_interface &src,
         const System &sys,
         std::pmr::memory_resource *rsrc) const -> result<value_type> {
    namespace updv = upd::tuple_views;

    using element_type = typename Description::value_type;

    auto len = solve_for(length_of<Id>, sys | updv::to<std::tuple>);
    if (len < 0) {
      return std::unexpected{
          negative_repetition_count{Id.string, static_cast<word_t>(len)}};
    }

    auto retval = std::vector<element_type>{};
    auto i = 0uz;
    while (i < static_cast<std::size_t>(len)) {
      auto maybe_value = description.template decode<Id>(src, sys, rsrc);
      if (!maybe_value) {
        return std::unexpected{std::move(maybe_value).error()};
      }
      retval.push_back(std::move(maybe_value).value());
      i += description.bitsize(retval.back());
    }

    return value_type{std::move(retval)};
  }

  [[nodiscard]] constexpr auto
  bitsize(const value_type &fseg) const -> std::size_t {
    auto retval = 0zu;
    for (const auto &elem : fseg) {
      retval += description.bitsize(elem);
    }

    return retval;
  }

  [[nodiscard]] constexpr auto
  bitsize(const input_type &dcast) const -> std::size_t {
    auto rg = dcast.template cast_to<typename description_type::input_type>();
    auto retval = 0zu;
    for (const auto &elem : rg) {
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

  return repeat_t<codec_type, rule_type>{
      .description = UPD_FWD(cdc),
      .rule = UPD_FWD(rule),
  };
}

template<record_like Codecs>
[[nodiscard]] constexpr auto repeat(Codecs &&cdcs) {
  return repeat(description{UPD_FWD(cdcs)}, unit);
}

template<record_like Codecs, typename Rule>
[[nodiscard]] constexpr auto repeat(Codecs &&cdcs, Rule &&rule) {
  return repeat(description{UPD_FWD(cdcs)}, UPD_FWD(rule));
}

} // namespace upd::descriptor
