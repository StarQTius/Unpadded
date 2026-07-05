#pragma once

#include <cstddef>

#include "../upd.hpp"
#include "../utility/constexpr.hpp"
#include "entry.hpp"
#include "get_ith.hpp"
#include "record_like.hpp"
#include "record_size.hpp"
#include "record_tag.hpp"

namespace upd {

template<std::size_t I, record_like Record>
  requires(I < record_size_v<Record>)
[[nodiscard]] constexpr auto get_ith_entry(Record &&rec) noexcept(release) {
  constexpr auto tag = record_tag_v<I, Record>;
  return entry{expr<tag>, get_ith<I>(UPD_FWD(rec))};
}

} // namespace upd
