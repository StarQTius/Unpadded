//! \file

#pragma once

#include "../../type.hpp"
#include "../../upd.hpp"
#include "../type_traits/require.hpp"

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
  template<typename Dest_F, UPD_REQUIREMENT(output_invocable, Dest_F)>
  void operator>>(Dest_F &&dest) {
    derived().write_to(UPD_FWD(dest));
  }

  template<typename Dest_F, UPD_REQUIREMENT(output_invocable, Dest_F)>
  void operator>>(Dest_F &&dest) const {
    derived().write_to(UPD_FWD(dest));
  }

  template<typename It, UPD_REQUIREMENT(output_byte_iterator, It)>
  void write_to(It it) {
    derived().write_to([&](byte_t byte) { *it++ = byte; });
  }

  template<typename It, UPD_REQUIREMENT(output_byte_iterator, It)>
  void write_to(It it) const {
    derived().write_to([&](byte_t byte) { *it++ = byte; });
  }

  template<typename It, UPD_REQUIREMENT(output_byte_iterator, It)>
  void operator>>(It src) {
    write_to(src);
  }

  template<typename It, UPD_REQUIREMENT(output_byte_iterator, It)>
  void operator>>(It src) const {
    write_to(src);
  }
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
//! \tparam Dest Byte putter type
//! \tparam It Output iterator type

} // namespace detail
} // namespace upd
