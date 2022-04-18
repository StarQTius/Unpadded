//! \file

#pragma once

#include <upd/type.hpp>

#include "../type_traits/require.hpp"
#include "immediate_reader.hpp"

namespace upd {
namespace detail {

//! \brief CRTP base class used to define reading members functions
//! \details
//!   Readers are able to read a byte sequence byte per byte.
//!   Derived classes must define a 'read' invocable on an input functor.
template<typename D, typename R>
class reader : public immediate_reader<D, R> {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  using immediate_reader<D, R>::operator<<;

  //! \name Reading functions
  //! \brief Call the 'read' member function of the derived class
  //! \details
  //!   These functions may be invoked on hardware registers and input iterators
  //! @{

  R read(const volatile byte_t &reg) {
    return derived().read([&]() { return reg; });
  }

  R read(const volatile byte_t &reg) const {
    return derived().read([&]() { return reg; });
  }

  R operator<<(const volatile byte_t &reg) {
    return derived().read([&]() { return reg; });
  }

  R operator<<(const volatile byte_t &reg) const {
    return derived().read([&]() { return reg; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  R read(It it) {
    return derived().read([&]() { return *it; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  R read(It it) const {
    return derived().read([&]() { return *it; });
  }

  //! @}
};

//! \class Reader_CRTP
//!
//! The following member functions are also defined through CRTP.
//! \code
//! R read(const volatile byte_t &);
//! R read(const volatile byte_t &) const;
//! R operator<<(const volatile byte_t &);
//! R operator<<(const volatile byte_t &) const;
//! R read(It it);
//! R read(It it) const;
//! \endcode
//! All these functions work the same way as `read(Src &&)`.
//!
//! \tparam It Input iterator type

} // namespace detail
} // namespace upd
