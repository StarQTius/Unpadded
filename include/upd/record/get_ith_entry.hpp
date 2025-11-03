#pragma once

#include <cstddef>
#include <type_traits>

#include "../constexpr.hpp"
#include "../upd.hpp"
#include "concepts.hpp"
#include "entry.hpp"
#include "get_ith.hpp"

namespace upd {

template<std::size_t I, record_like Record>
  requires(I < record_size_v<std::remove_cvref_t<Record>>)
[[nodiscard]] constexpr auto get_ith_entry(Record &&rec) noexcept(release) {
  using record_type = std::remove_cvref_t<Record>;
  constexpr auto tag = record_tag_v<I, record_type>;
  return entry{expr<tag>, get_ith<I>(UPD_FWD(rec))};
}

} // namespace upd
