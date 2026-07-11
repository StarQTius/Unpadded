#pragma once

#include <array>
#include <concepts>
#include <ranges>
#include <type_traits>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "../utility/type_traits.hpp"
#include "enumerate.hpp"
#include "filter.hpp"
#include "to.hpp"
#include "transform.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd::tuple_views {

constexpr auto find_if = []<tuple_like2 Tuple>(Tuple &&t, auto &&pred) {
  auto p = [pred]<typename IAndTypebox>(typebox<IAndTypebox>) constexpr {
    using value_type = typename std::remove_cvref_t<IAndTypebox>::second_type;
    return UPD_INVOKE(pred, typebox<value_type>{});
  };

  auto vw = UPD_FWD(t) | enumerate | filter(p);
  if constexpr (tuple_size_v<decltype(vw)> > 0) {
    return get<0>(vw).first;
  } else {
    return expr<tuple_size_v<Tuple>>;
  }
};

template<typename T>
constexpr auto find = []<tuple_like2 Tuple>(Tuple &&t) {
  auto p = []<typename IAndTypebox>(typebox<IAndTypebox>) {
    return std::same_as<typename std::remove_cvref_t<IAndTypebox>::second_type,
                        T>;
  };
  auto vw = UPD_FWD(t) | enumerate | filter(p);

  if constexpr (tuple_size_v<decltype(vw)> > 0) {
    return get<0>(vw).first;
  } else {
    return expr<tuple_size_v<Tuple>>;
  }
};

constexpr auto dynfind = []<tuple_like2 Tuple, typename T>(Tuple &&t,
                                                           const T &v) {
  namespace stdr = std::ranges;

  auto equal_elements = UPD_FWD(t)
                        | transform([&]<typename U>(const U &x) {
                            if constexpr (std::equality_comparable_with<T, U>) {
                              return x == v;
                            } else {
                              return false;
                            }
                          })
                        | to<std::array>;

  return stdr::find(equal_elements, true) - equal_elements.begin();
};

} // namespace upd::tuple_views
