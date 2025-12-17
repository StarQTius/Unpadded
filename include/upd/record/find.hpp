#pragma once

#include <concepts>
#include <type_traits>

#include "../constexpr.hpp"
#include "../equivalent_to.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "enumerate.hpp"
#include "filter.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"

namespace upd::record_views {

constexpr auto find_if = []<record_like Record>(Record &&rec, auto &&pred) {
  auto p = [pred]<typename IAndTypebox>(auto k, typebox<IAndTypebox>) constexpr {
    using value_type = typename std::remove_cvref_t<IAndTypebox>::second_type;
    return UPD_INVOKE(pred, k, typebox<value_type>{});
  };

  auto vw = UPD_FWD(rec) | enumerate | filter(p);
  if constexpr (record_size_v<decltype(vw)> > 0) {
    return get_ith<0>(vw).first;
  } else {
    return expr<record_size_v<Record>>;
  }
};

template<typename T>
constexpr auto find_type = []<record_like Record>(Record &&rec) {
  auto p = []<typename IAndTypebox>(auto, typebox<IAndTypebox>) {
    return std::same_as<typename std::remove_cvref_t<IAndTypebox>::second_type, T>;
  };

  auto vw = UPD_FWD(rec) | enumerate | filter(p);
  if constexpr (record_size_v<decltype(vw)> > 0) {
    return get_ith<0>(vw).first;
  } else {
    return expr<record_size_v<Record>>;
  }
};

template<auto Tag>
constexpr auto find_tag = []<record_like Record>(Record &&rec) {
  auto p = []<typename IAndTypebox>(auto k, typebox<IAndTypebox>) { return equivalent_to<k.value, Tag>; };

  auto vw = UPD_FWD(rec) | enumerate | filter(p);
  if constexpr (record_size_v<decltype(vw)> > 0) {
    return get_ith<0>(vw).first;
  } else {
    return expr<record_size_v<Record>>;
  }
};

} // namespace upd::record_views
