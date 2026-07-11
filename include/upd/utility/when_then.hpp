#pragma once

#include <type_traits>

#include "../record/entry.hpp"
#include "../record/record.hpp"
#include "../upd.hpp"

namespace upd {

template<auto Match, typename Result>
struct when_then_t {
  constexpr static auto match = Match;

  using result_type = Result;

  result_type result;
};

template<auto Match>
struct when_t {
  constexpr static auto match = Match;

  template<typename Result>
  [[nodiscard]] constexpr auto operator=(Result &&result) const {
    using result_type = std::decay_t<Result>;
    return when_then_t<match, result_type>{UPD_FWD(result)};
  }
};

template<auto Match>
constexpr auto when = when_t<Match>{};

template<typename... WhenThens>
[[nodiscard]] constexpr auto aggregate_when_thens(WhenThens &&...when_thens) {
  return record{entry<when_thens.match, typename WhenThens::result_type>{
      UPD_FWD(when_thens).result}...};
}

} // namespace upd
