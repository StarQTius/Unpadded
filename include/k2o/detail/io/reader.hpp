//! \file

#pragma once

#include <upd/type.hpp>

#include "../type_traits/require.hpp"
#include "immediate_reader.hpp"

namespace k2o {
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

  void read(const volatile upd::byte_t &reg) {
    derived().read([&]() { return reg; });
  }

  void read(const volatile upd::byte_t &reg) const {
    derived().read([&]() { return reg; });
  }

  void operator<<(const volatile upd::byte_t &reg) {
    derived().read([&]() { return reg; });
  }

  void operator<<(const volatile upd::byte_t &reg) const {
    derived().read([&]() { return reg; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  void read(It it) {
    derived().read([&]() { return *it; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  void read(It it) const {
    derived().read([&]() { return *it; });
  }

  //! @}
};

//! \class Reader_CRTP
//!
//! The following member functions are also defined through CRTP.
//! \code
//! void read(const volatile upd::byte_t &);
//! void read(const volatile upd::byte_t &) const;
//! void operator<<(const volatile upd::byte_t &);
//! void operator<<(const volatile upd::byte_t &) const;
//! void read(It it);
//! void read(It it) const;
//! \endcode
//! All these functions work the same way as `read(Src &&)`.
//!
//! \tparam It Input iterator type

} // namespace detail
} // namespace k2o
