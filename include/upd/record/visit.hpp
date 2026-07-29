#pragma once

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "../tuple/apply.hpp"
#include "../tuple/find.hpp"
#include "../tuple/instantiate.hpp"
#include "../tuple/typelist.hpp"
#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/with_sequence.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"
#include "tags_of.hpp"
#include "transform.hpp"
#include "values.hpp"

namespace upd::record_views {

constexpr auto visit =
    []<record_like Record, typename Tag, typename F> [[nodiscard]] (
        Record &&rec, const Tag &tag, F &&f) -> decltype(auto) {
  using alternative_types =
      decltype(rec
               | transform_type(
                   []<auto K, typename T> -> decltype(UPD_INVOKE_TEMPLATE(
                                              std::declval<F>(), (K),
                                              std::declval<T>())) {})
               | values
               | tuple_views::instantiate<typelist2_t>);

  using retval_type = decltype(tuple_views::apply_type(
      alternative_types{}, []<typename... Ts> -> std::common_type_t<Ts...> {}));

  auto i = tuple_views::dynfind(tags_of_v<Record>, tag);
  UPD_ASSERT(i < record_size_v<Record>);

  auto make_invoker_for_ith = []<std::size_t I>(expr_t<I>) {
    return +[](Record &&rec, F &&f) -> retval_type {
      return UPD_INVOKE_TEMPLATE(UPD_FWD(f), (record_tag_v<I, Record>),
                                 get_ith<I>(UPD_FWD(rec)));
    };
  };

  auto lut = UPD_WITH_SEQUENCE(Is, record_size_v<Record>, &) {
    return std::array{make_invoker_for_ith(expr<Is>)...};
  };

  return UPD_INVOKE(lut[i], UPD_FWD(rec), UPD_FWD(f));
};

} // namespace upd::record_views
