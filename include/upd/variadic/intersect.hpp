#pragma once

#include <tuple>
#include <type_traits>

#include "../record/lite_record.hpp"
#include "../tuple/concat.hpp"
#include "../tuple/filter.hpp"
#include "../tuple/has_type.hpp"
#include "../tuple/to.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "../utility/type_traits.hpp"
#include "../utility/with_sequence.hpp"

namespace upd {

template<metavalue... Xs, metavalue... Ys>
[[nodiscard]] constexpr auto intersect(std::tuple<Xs...> lhs, std::tuple<Ys...> rhs) noexcept(release) {
  namespace updv = upd::tuple_views;

  auto merged_view = updv::concat(lhs, rhs);
  auto merged = UPD_WITH_SEQUENCE(Is, sizeof...(Xs) + sizeof...(Ys), &) {
    return lite_record{lite_record_node{get<Is>(merged_view), expr<Is>}...};
  };

  auto inter_with_dup =
      merged_view
      | updv::filter([&]<typename T>(typebox<T>) { return !requires { merged.get_by_tag(std::decay_t<T>{}); }; })
      | updv::to<std::tuple>;

  return lhs
         | updv::filter(
             [&]<typename T>(typebox<T>) { return tuple_has_type_v<std::decay_t<T>, decltype(inter_with_dup)>; })
         | updv::to<std::tuple>;
}

} // namespace upd
