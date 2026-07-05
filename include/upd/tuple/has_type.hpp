#pragma once

#include <concepts>

#include "../upd.hpp"
#include "../utility/with_sequence.hpp"
#include "tuple_element.hpp"
#include "tuple_like.hpp"
#include "tuple_size.hpp"

namespace upd {

template<typename T, tuple_like2 Tuple>
struct tuple_has_type {
  constexpr static auto value = UPD_WITH_SEQUENCE(Is, tuple_size_v<Tuple>) {
    return (std::same_as<tuple_element_t<Is, Tuple>, T> || ...);
  };
};

template<typename T, tuple_like2 Tuple>
constexpr auto tuple_has_type_v = tuple_has_type<T, Tuple>::value;

template<typename T, tuple_like2 Tuple>
[[nodiscard]] constexpr auto has_type(const Tuple &) noexcept(release) -> bool {
  return tuple_has_type_v<T, Tuple>;
}

} // namespace upd
