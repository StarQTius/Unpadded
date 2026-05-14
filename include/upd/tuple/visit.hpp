#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

#include "../concept/invocable.hpp"
#include "../constexpr.hpp"
#include "../get.hpp"
#include "../tuple/instantiate.hpp"
#include "../tuple/typelist.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "apply.hpp"
#include "transform.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd::tuple_views {

constexpr auto visit =
    []<tuple_like2 Tuple, typename F> [[nodiscard]] (Tuple &&t, std::size_t i, F &&f) -> decltype(auto) {
  using xxx = decltype(t
                       | transform_type([]<typename T>
                                          requires invocable<F, T>
                                        -> std::invoke_result<F, T> {})
                                        | instantiate<typelist2_t>);

  using retval_type =
      decltype(apply_type(xxx{}, []<typename... Metatype> -> std::common_type_t<typename Metatype::type...> {}));

  UPD_ASSERT(i < tuple_size_v<Tuple>);

  auto make_invoker_for_ith = []<std::size_t I>(expr_t<I>) {
    return +[](Tuple &&t, F &&f) -> retval_type { return UPD_INVOKE(UPD_FWD(f), get<I>(UPD_FWD(t))); };
  };

  auto lut = UPD_WITH_SEQUENCE(Is, tuple_size_v<Tuple>, &) { return std::array{make_invoker_for_ith(expr<Is>)...}; };

  return UPD_INVOKE(lut[i], UPD_FWD(t), UPD_FWD(f));
};

} // namespace upd::tuple_views
