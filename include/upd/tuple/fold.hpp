#pragma once

#include "../upd.hpp"
#include "../utility/get.hpp"
#include "../utility/with_sequence.hpp"
#include "../variadic/folder.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd::tuple_views {

constexpr auto fold_left =
    []<tuple_like2 Tuple>(Tuple &&t, auto &&init, auto &&op) -> decltype(auto) {
  auto f = [&](auto &&acc, auto &&x) {
    return UPD_INVOKE(op, UPD_FWD(acc), UPD_FWD(x));
  };

  return UPD_WITH_SEQUENCE(Is, tuple_size_v<Tuple>, &) {
    return (UPD_FWD(init), ..., variadic::folder(get<Is>(UPD_FWD(t)), f));
  };
};

constexpr auto fold_right =
    []<tuple_like2 Tuple>(Tuple &&t, auto &&init, auto &&op) -> decltype(auto) {
  auto f = [&](auto &&x, auto &&acc) {
    return UPD_INVOKE(op, UPD_FWD(x), UPD_FWD(acc));
  };

  return UPD_WITH_SEQUENCE(Is, tuple_size_v<Tuple>, &) {
    return (variadic::folder(get<Is>(UPD_FWD(t)), f), ..., UPD_FWD(init));
  };
};

} // namespace upd::tuple_views
