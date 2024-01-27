#pragma once

#include <cstddef>

#include "always_false.hpp"
#include "type_traits/is_array.hpp"

namespace upd::detail {

template<typename T>
constexpr auto serialized_size() noexcept -> std::size_t {
  if constexpr (std::is_integral_v<T> || detail::is_array<T>::value) {
    return sizeof(T);
  } else {
    static_assert(UPD_ALWAYS_FALSE, "Type is not serializable");
  }
}

} // namespace upd::detail
