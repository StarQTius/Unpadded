#pragma once

#include <cstddef>

#include "../upd.hpp"
#include "concepts.hpp"
#include "record_tag.hpp"

namespace upd {

template<std::size_t I, record_like Record>
[[nodiscard]] constexpr static auto get_ith(Record &&rec) -> decltype(auto) {
  constexpr auto tag = record_tag_v<I, Record>;
  return get<tag>(UPD_FWD(rec));
}

} // namespace upd
