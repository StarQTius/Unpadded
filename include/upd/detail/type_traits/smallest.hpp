//! \file

#pragma once

#include <cstdint>
#include <type_traits>

#include "require.hpp"

namespace upd {
namespace detail {

template<typename T, unsigned long long X>
struct smallest_unsigned_impl {
  using type = T;

  template<typename U, detail::require<U(X) == X>>
  smallest_unsigned_impl<U, X> try_promote(U &&) const;
  smallest_unsigned_impl<T, X> try_promote(...) const;
};

//! \name
//! \brief Gets the smallest unsigned integer type that can hold `X`
//! @{

template<unsigned long long X>
struct smallest_unsigned {
  using type = typename decltype(smallest_unsigned_impl<std::uint8_t, X>{}
                                     .template try_promote(std::declval<std::uint16_t>())
                                     .template try_promote(std::declval<std::uint32_t>())
                                     .template try_promote(std::declval<std::uint64_t>()))::type;
};

template<unsigned long long X>
using smallest_unsigned_t = typename smallest_unsigned<X>::type;

//! @}

} // namespace detail
} // namespace upd
