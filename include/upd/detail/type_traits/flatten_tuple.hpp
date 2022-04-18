//! \file

#pragma once

#include <upd/format.hpp>
#include <upd/tuple.hpp>

namespace upd {
namespace detail {

//! \name
//! \brief Flatten a tuple type if needed
//!
//! If `Ts` expands to a single type, and that type can be expressed as `tuple<...>`, then `flatten_tuple_t<Ts...>`
//! is an alias of `Ts...`. If `Ts...` expands into `void`, then `flatten_tuple_t<Ts...>` is an alias of
//! tuple<Endianess, Signed_Mode>. Otherwise, type `flatten_tuple_t<Ts...>` is an alias of `tuple<Endianess,
//! Signed_Mode, Ts...>`.
//! @{

template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
struct flatten_tuple {
  using type = tuple<Endianess, Signed_Mode, Ts...>;
};
template<endianess Endianess,
         signed_mode Signed_Mode,
         endianess Endianess_Tuple,
         signed_mode Signed_Mode_Tuple,
         typename... Ts>
struct flatten_tuple<Endianess, Signed_Mode, tuple<Endianess_Tuple, Signed_Mode_Tuple, Ts...>> {
  using type = tuple<Endianess_Tuple, Signed_Mode_Tuple, Ts...>;
};
template<endianess Endianess, signed_mode Signed_Mode>
struct flatten_tuple<Endianess, Signed_Mode, void> {
  using type = tuple<Endianess, Signed_Mode>;
};

template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
using flatten_tuple_t = typename flatten_tuple<Endianess, Signed_Mode, Ts...>::type;

//! @}

} // namespace detail
} // namespace upd
