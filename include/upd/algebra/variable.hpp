#pragma once

#include "../record/concepts.hpp"
#include "../record/lite_record.hpp"
#include "../upd.hpp"

namespace upd::algebra {

template<auto Name>
struct variable {
  constexpr static auto name = Name;

  [[nodiscard]] constexpr auto operator==(variable<Name>) noexcept(release) -> bool { return true; }

  template<auto VN>
  [[nodiscard]] constexpr auto operator==(variable<VN>) noexcept(release) -> bool {
    return false;
  }
};

template<auto Varname, record_like Lets>
[[nodiscard]] constexpr auto substitute(variable<Varname>, const Lets &lets) noexcept(release) {
  if constexpr (has_tag<Varname>(lets)) {
    return get<Varname>(lets);
  } else {
    return variable<Varname>{};
  }
}

template<auto Varname, auto OtherVarname>
[[nodiscard]] constexpr auto depends_on(variable<OtherVarname> var) noexcept(release) -> bool {
  return variable<Varname>{} == var;
}

} // namespace upd::algebra
