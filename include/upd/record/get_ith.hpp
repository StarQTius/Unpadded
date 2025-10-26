#pragma once

#include <cstddef>
#include <type_traits>

#include "../upd.hpp"
#include "concepts.hpp"

namespace upd {

template<std::size_t I, record_like Record>
[[nodiscard]] constexpr static auto get_ith(Record &&rec) -> decltype(auto) {
  constexpr auto tag = record_tag_v<I, std::remove_cvref_t<Record>>;
  return get<tag>(UPD_FWD(rec));
}

} // namespace upd
