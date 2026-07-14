#pragma once

#include <concepts>
#include <type_traits>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/equivalent_to.hpp"
#include "enumerate.hpp"
#include "filter.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"

namespace upd::record_views {

constexpr auto find_if = []<record_like Record>(Record &&rec, auto &&pred) {
  auto p = [pred]<auto Tag, typename IndexedType> {
    using type = std::remove_cvref_t<typename IndexedType::second_type>;
    return UPD_INVOKE_TEMPLATE(pred, (Tag, type));
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
  auto p = []<auto, typename IndexedType> {
    using type = std::remove_cvref_t<typename IndexedType::second_type>;
    return std::same_as<type, T>;
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
  auto p = []<auto K, typename> { return equivalent_to<K, Tag>; };

  auto vw = UPD_FWD(rec) | enumerate | filter(p);
  if constexpr (record_size_v<decltype(vw)> > 0) {
    return get_ith<0>(vw).first;
  } else {
    return expr<record_size_v<Record>>;
  }
};

} // namespace upd::record_views
