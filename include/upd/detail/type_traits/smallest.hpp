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

  template<typename U, UPD_REQUIRE(U(X) == X)>
  smallest_unsigned_impl<U, X> try_promote(U &&) const;
  template<typename U, UPD_REQUIRE(U(X) != X)>
  smallest_unsigned_impl<T, X> try_promote(U &&) const;
};

//! \name
//! \brief Gets the smallest unsigned integer type that can hold `X`
//! @{

template<unsigned long long X>
struct smallest_unsigned {
  using type = typename decltype(smallest_unsigned_impl<std::uint64_t, X>{}
                                     .template try_promote(std::declval<std::uint32_t>())
                                     .template try_promote(std::declval<std::uint16_t>())
                                     .template try_promote(std::declval<std::uint8_t>()))::type;
};

template<unsigned long long X>
using smallest_unsigned_t = typename smallest_unsigned<X>::type;

//! @}

} // namespace detail
} // namespace upd
