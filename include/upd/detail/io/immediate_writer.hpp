//! \file

#pragma once

#include "../../type.hpp"
#include "../type_traits/require.hpp"

#include "../def.hpp"

namespace upd {
namespace detail {

//! \brief CRTP base class used to define immediate writing members functions
//! \details
//!   Immediate writers are able to write a byte sequence completely.
//!   Derived classes must define a `write_to` invocable on an input functor.
template<typename D>
class immediate_writer {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  //! \name Immediate writing functions
  //! \brief Call the `write_to` member function of the derived class
  //! \details
  //!   These functions may be invoked on hardware registers and output functors and iterators
  //! @{

  template<typename Dest_F, detail::require_output_invocable<Dest_F> = 0>
  void operator>>(Dest_F &&dest) {
    derived().write_to(FWD(dest));
  }

  template<typename Dest_F, detail::require_output_invocable<Dest_F> = 0>
  void operator>>(Dest_F &&dest) const {
    derived().write_to(FWD(dest));
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  void write_to(It it) {
    derived().write_to([&](byte_t byte) { *it++ = byte; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  void write_to(It it) const {
    derived().write_to([&](byte_t byte) { *it++ = byte; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  void operator>>(It src) {
    write_to(src);
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  void operator>>(It src) const {
    write_to(src);
  }

  //! @}
};

//! \class ImmediateWriter_CRTP
//!
//! The following member functions are also defined through CRTP.
//! \code
//! void operator>>(Dest &&);
//! void operator>>(Dest &&) const;
//! void write_to(It);
//! void write_to(It) const;
//! void operator>>(It);
//! void operator>>(It) const;
//! \endcode
//! All these functions work the same way as `write_to(Dest &&)`.
//!
//! \tparam Dest Output functor type
//! \tparam It Output iterator type

} // namespace detail
} // namespace upd

#include "../undef.hpp" // IWYU pragma: keep
