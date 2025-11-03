#pragma once

#include <type_traits>

#include "../functional.hpp"
#include "../upd.hpp"
#include "../variadic/folder.hpp"
#include "../with_sequence.hpp"
#include "concepts.hpp"
#include "get_ith_entry.hpp"

namespace upd::record_views {

constexpr auto fold_left = []<record_like Record>(Record &&rec, auto &&init, auto &&op) -> decltype(auto) {
  using record_type = std::remove_cvref_t<Record>;
  auto f = [&](auto &&acc, auto &&ent) { return UPD_INVOKE(op, UPD_FWD(acc), ent.identifier, UPD_FWD(ent).value); };

  return UPD_WITH_SEQUENCE(Is, record_size_v<record_type>, &) {
    return (UPD_FWD(init), ..., variadic::folder(get_ith_entry<Is>(UPD_FWD(rec)), f));
  };
};

constexpr auto fold_right = []<record_like Record>(Record &&rec, auto &&init, auto &&op) -> decltype(auto) {
  using record_type = std::remove_cvref_t<Record>;
  auto f = [&](auto &&ent, auto &&acc) { return UPD_INVOKE(op, ent.identifier, UPD_FWD(ent).value, UPD_FWD(acc)); };

  return UPD_WITH_SEQUENCE(Is, record_size_v<record_type>, &) {
    return (variadic::folder(get_ith_entry<Is>(UPD_FWD(rec)), f), ..., UPD_FWD(init));
  };
};

} // namespace upd::record_views
