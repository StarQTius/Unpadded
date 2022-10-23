//! \file

#pragma once

#include <type_traits>

namespace upd {

enum class endianess;
enum class signed_mode;

template<endianess, signed_mode, typename...>
struct tuple; // IWYU pragma: keep

template<typename, endianess, signed_mode, typename...>
struct tuple_view; // IWYU pragma: keep

namespace detail {

//! \name
//! \brief Indicates if the provided type is a template instance of `tuple` or `tuple_view`

template<typename>
struct is_tuple : std::false_type {};
template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
struct is_tuple<tuple<Endianess, Signed_Mode, Ts...>> : std::true_type {};
template<typename It, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
struct is_tuple<tuple_view<It, Endianess, Signed_Mode, Ts...>> : std::true_type {};

//! @}

} // namespace detail
} // namespace upd
