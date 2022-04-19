//! \file

#pragma once

#include <type_traits>

#include "../../format.hpp"

namespace upd {

enum class endianess;
enum class signed_mode;

template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
struct tuple; // IWYU pragma: keep

namespace detail {

//! \name
//! \brief Indicates if the provided type is a template instance of `tuple`

template<typename>
struct is_tuple : std::false_type {};
template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
struct is_tuple<tuple<Endianess, Signed_Mode, Ts...>> : std::true_type {};

//! @}

} // namespace detail
} // namespace upd
