//! \file

#pragma once

#include <upd/type.hpp>

#include "../type_traits/require.hpp"
#include "immediate_writer.hpp"

namespace upd {
namespace detail {

//! \brief CRTP base class used to define writing members functions
//! \details
//!   Writers are able to write a byte sequence byte per byte.
//!   Derived classes must define a 'write' invocable on an output functor.
template<typename D>
class writer : public immediate_writer<D> {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  //! \name Writing functions
  //! \brief Call the 'write' member function of the derived class
  //! \details
  //!   These functions may be invoked on hardware registers and output iterators
  //! @{

  void write(volatile byte_t &reg) {
    derived().write([&](byte_t byte) { reg = byte; });
  }

  void write(volatile byte_t &reg) const {
    derived().write([&](byte_t byte) { reg = byte; });
  }

  using immediate_writer<D>::operator>>;

  void operator>>(volatile byte_t &reg) {
    derived().write([&](byte_t byte) { reg = byte; });
  }

  void operator>>(volatile byte_t &reg) const {
    derived().write([&](byte_t byte) { reg = byte; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  void write(It it) {
    derived().write([&](byte_t byte) { *it = byte; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  void write(It it) const {
    derived().write([&](byte_t byte) { *it = byte; });
  }

  // @}
};

//! \class Writer_CRTP
//!
//! The following member functions are also defined through CRTP.
//! \code
//! void write(volatile byte_t &);
//! void write(volatile byte_t &) const;
//! void operator>>(volatile byte_t &);
//! void operator>>(volatile byte_t &) const;
//! void write(It);
//! void write(It) const;
//! \endcode
//! All these functions work the same way as `write(Dest &&)`.
//!
//! \tparam It Output iterator type

} // namespace detail
} // namespace upd
