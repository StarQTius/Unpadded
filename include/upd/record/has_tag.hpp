#pragma once

#include <concepts>

#include "../constexpr.hpp"
#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "concepts.hpp"

namespace upd {

template<auto Tag, record_like Record>
[[nodiscard]] constexpr auto has_tag(const Record &) noexcept(release) -> bool {
  return UPD_WITH_SEQUENCE(Is, record_size_v<Record>) {
    return (std::same_as<auto_constant<record_tag_v<Is, Record>>, auto_constant<Tag>> || ...);
  };
}

} // namespace upd
