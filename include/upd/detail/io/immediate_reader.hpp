//! \file

#pragma once

#include "../type_traits/require.hpp"

#include "../def.hpp"

namespace upd {
namespace detail {

//! \brief CRTP base class used to define immediate reading members functions
//! \details
//!   Immediate readers are able to read a byte sequence completely.
//!   Derived classes must define a 'read_all' invocable on an input functor.
template<typename D, typename R>
class immediate_reader {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  //! \name Immediate reading functions
  //! \brief Call the 'read_all' member function of the derived class
  //! \details
  //!   These functions may be invoked on hardware registers and input functors and iterators
  //! @{

  template<typename Src_F, detail::require_input_invocable<Src_F> = 0>
  R operator<<(Src_F &&src) {
    return derived().read_all(FWD(src));
  }

  template<typename Src_F, detail::require_input_invocable<Src_F> = 0>
  R operator<<(Src_F &&src) const {
    return derived().read_all(FWD(src));
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  R read_all(It it) {
    return derived().read_all([&]() { return *it++; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  R read_all(It it) const {
    return derived().read_all([&]() { return *it++; });
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  R operator<<(It src) {
    return read_all(src);
  }

  template<typename It, detail::require_byte_iterator<It> = 0>
  R operator<<(It src) const {
    return read_all(src);
  }

  //! @}
};

//! \class ImmediateReader_CRTP
//!
//! The following member functions are also defined through CRTP.
//! \code
//! auto operator<<(Src &&);
//! auto operator<<(Src &&) const;
//! auto read_all(It);
//! auto read_all(It) const;
//! auto operator<<(It);
//! auto operator<<(It) const;
//! \endcode
//! All these functions work the same way as `read_all(Src &&)`.
//!
//! \tparam Src Input functor type
//! \tparam It Input iterator type

} // namespace detail
} // namespace upd

#include "../undef.hpp" // IWYU pragma: keep
