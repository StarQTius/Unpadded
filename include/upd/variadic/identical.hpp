#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <tuple>

#include "../record/lite_record.hpp"
#include "../tuple/concat.hpp"
#include "../tuple/to.hpp"
#include "../tuple/transform.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "../utility/type_traits.hpp"
#include "../utility/with_sequence.hpp"

namespace upd::variadic {

template<metavalue... Xs, metavalue... Ys>
[[nodiscard]] constexpr auto
identical(std::tuple<Xs...> lhs, std::tuple<Ys...> rhs) noexcept(release)
    -> bool {
  namespace stdr = std::ranges;
  namespace updv = upd::tuple_views;

  auto merged_view = updv::concat(lhs, rhs);
  auto merged = UPD_WITH_SEQUENCE(Is, sizeof...(Xs) + sizeof...(Ys), &) {
    return lite_record{lite_record_node{get<Is>(merged_view), expr<Is>}...};
  };

  auto is_dup = merged_view
                | updv::transform(
                    [&](auto x) { return !requires { merged.get_by_tag(x); }; })
                | updv::to<std::array>;

  return stdr::all_of(is_dup, std::identity{});
}

} // namespace upd::variadic
