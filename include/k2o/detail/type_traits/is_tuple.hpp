//! \file

#pragma once

#include <type_traits>

#include <upd/format.hpp>
#include <upd/tuple.hpp>

namespace k2o {
namespace detail {

//! \name
//! \brief Indicates if the provided type is a template instance of `upd::tuple`

template<typename>
struct is_tuple : std::false_type {};
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct is_tuple<upd::tuple<Endianess, Signed_Mode, Ts...>> : std::true_type {};

//! @}

} // namespace detail
} // namespace k2o
