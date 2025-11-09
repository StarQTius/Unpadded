#pragma once

#include <concepts>

#include "../upd.hpp"
#include "../with_sequence.hpp"
#include "ith_record_element.hpp"
#include "record_like.hpp"
#include "record_size.hpp"

namespace upd {

template<typename T, record_like Record>
[[nodiscard]] constexpr auto has_type(const Record &) noexcept(release) -> bool {
  return UPD_WITH_SEQUENCE(Is, record_size_v<Record>) {
    return (std::same_as<ith_record_element_t<Is, Record>, T> || ...);
  };
}

} // namespace upd
