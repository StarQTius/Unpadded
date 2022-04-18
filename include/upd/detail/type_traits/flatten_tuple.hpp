//! \file

#pragma once

#include <upd/format.hpp>
#include <upd/tuple.hpp>

namespace k2o {
namespace detail {

//! \name
//! \brief Flatten a tuple type if needed
//!
//! If `Ts` expands to a single type, and that type can be expressed as `upd::tuple<...>`, then `flatten_tuple_t<Ts...>`
//! is an alias of `Ts...`. If `Ts...` expands into `void`, then `flatten_tuple_t<Ts...>` is an alias of
//! upd::tuple<Endianess, Signed_Mode>. Otherwise, type `flatten_tuple_t<Ts...>` is an alias of `upd::tuple<Endianess,
//! Signed_Mode, Ts...>`.
//! @{

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct flatten_tuple {
  using type = upd::tuple<Endianess, Signed_Mode, Ts...>;
};
template<upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         upd::endianess Endianess_Tuple,
         upd::signed_mode Signed_Mode_Tuple,
         typename... Ts>
struct flatten_tuple<Endianess, Signed_Mode, upd::tuple<Endianess_Tuple, Signed_Mode_Tuple, Ts...>> {
  using type = upd::tuple<Endianess_Tuple, Signed_Mode_Tuple, Ts...>;
};
template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct flatten_tuple<Endianess, Signed_Mode, void> {
  using type = upd::tuple<Endianess, Signed_Mode>;
};

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
using flatten_tuple_t = typename flatten_tuple<Endianess, Signed_Mode, Ts...>::type;

//! @}

} // namespace detail
} // namespace k2o
