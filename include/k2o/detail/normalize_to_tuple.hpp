//! \file
//! \brief Type normalization to 'upd::tuple'

#pragma once

#include <upd/tuple.hpp>

namespace k2o {
namespace detail {

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct normalize_to_tuple {
  using type = upd::tuple<Endianess, Signed_Mode, Ts...>;
};

template<upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         upd::endianess Endianess_Tuple,
         upd::signed_mode Signed_Mode_Tuple,
         typename... Ts>
struct normalize_to_tuple<Endianess, Signed_Mode, upd::tuple<Endianess_Tuple, Signed_Mode_Tuple, Ts...>> {
  using type = upd::tuple<Endianess_Tuple, Signed_Mode_Tuple, Ts...>;
};

template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct normalize_to_tuple<Endianess, Signed_Mode, void> {
  using type = upd::tuple<Endianess, Signed_Mode>;
};

//! \brief Normalize a type to tuple if needed
//! \details
//!   If 'Ts' expands into a single type, and that type can be expressed as 'upd::tuple<...>', then
//!   'normalize_to_tuple_t' is an alias of 'Ts...'. If 'Ts...' expands into 'void', then 'normalize_to_tuple_t' is an
//!   alias of upd::tuple<Endianess, Signed_Mode>. Otherwise, type 'normalize_to_tuple_t' is an alias of
//!   'upd::tuple<Endianess, Signed_Mode, Ts...>'.
//! \tparam Endianess endianess parameter for 'upd::tuple'
//! \tparam Signed_Mode signed representation parameter for 'upd::tuple'
//! \tparam Ts... considered parameter pack
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
using normalize_to_tuple_t = typename normalize_to_tuple<Endianess, Signed_Mode, Ts...>::type;

} // namespace detail
} // namespace k2o
