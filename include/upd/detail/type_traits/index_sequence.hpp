//! \file

#pragma once

#include <cstddef>

#if __cplusplus >= 201402L
#include <utility>
#endif // __cplusplus >= 201402L

// NOLINTBEGIN

namespace upd {
namespace detail {

#if __cplusplus >= 201402L

//! \brief Alias for `std::index_sequence`
template<std::size_t... Is>
using index_sequence = std::index_sequence<Is...>;

//! \brief Alias for `std::make_index_sequence`
template<std::size_t I>
using make_index_sequence = std::make_index_sequence<I>;

#else // __cplusplus >= 201402L

//! \brief Holds a list of compile-time indices
template<std::size_t... Is>
struct index_sequence {};

//! \name
//! \brief Alias for `std::index_sequence</*0, 1, 2, ..., I*/>`
//! @{

template<std::size_t I, std::size_t... Is>
struct make_index_sequence : make_index_sequence<I - 1, I - 1, Is...> {};
template<std::size_t... Is>
struct make_index_sequence<0, Is...> : index_sequence<Is...> {};

//! @}

#endif // __cplusplus >= 201402L

} // namespace detail
} // namespace upd

// NOLINTEND
