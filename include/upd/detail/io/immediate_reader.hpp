//! \file

#pragma once

#include "../../upd.hpp"
#include "../type_traits/require.hpp"

#include "../def.hpp"

namespace upd {
namespace detail {

//! \brief CRTP base class used to define immediate reading members functions
//! \details
//!   Immediate readers are able to read a byte sequence completely.
//!   Derived classes must define a `read_from` invocable on an input functor.
template<typename D, typename R>
class immediate_reader {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  //! \name Immediate reading functions
  //! \brief Call the `read_from` member function of the derived class
  //! \details
  //!   These functions may be invoked on hardware registers and input functors and iterators
  //! @{

  template<typename Src_F, UPD_REQUIREMENT(input_invocable, Src_F)>
  R operator<<(Src_F &&src) {
    return derived().read_from(FWD(src));
  }

  template<typename Src_F, UPD_REQUIREMENT(input_invocable, Src_F)>
  R operator<<(Src_F &&src) const {
    return derived().read_from(FWD(src));
  }

  template<typename It, UPD_REQUIREMENT(input_byte_iterator, It)>
  R read_from(It it) {
    return derived().read_from([&]() { return *it++; });
  }

  template<typename It, UPD_REQUIREMENT(input_byte_iterator, It)>
  R read_from(It it) const {
    return derived().read_from([&]() { return *it++; });
  }

  template<typename It, UPD_REQUIREMENT(input_byte_iterator, It)>
  R operator<<(It src) {
    return read_from(src);
  }

  template<typename It, UPD_REQUIREMENT(input_byte_iterator, It)>
  R operator<<(It src) const {
    return read_from(src);
  }

  //! @}
};

//! \class ImmediateReader_CRTP
//!
//! The following member functions are also defined through CRTP.
//! \code
//! R operator<<(Src &&);
//! R operator<<(Src &&) const;
//! R read_from(It);
//! R read_from(It) const;
//! R operator<<(It);
//! R operator<<(It) const;
//! \endcode
//! All these functions work the same way as `read_from(Src &&)`.
//!
//! \tparam Src Input functor type
//! \tparam It Input iterator type

} // namespace detail
} // namespace upd

#include "../undef.hpp" // IWYU pragma: keep
