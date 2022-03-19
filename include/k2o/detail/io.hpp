//! \file
//! \brief Input / output utilities

#pragma once

#include "sfinae.hpp"

#include "def.hpp"

namespace k2o {
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
  //! \k2o_doc{DoubleLeftArrow_Functor}
  //! \brief Call the 'read_all' member function of derived class
  //! \param src Input functor to the byte sequence to read
  //! \return the resulting value of the 'read_all' call

  //! \copydoc DoubleLeftArrow_Functor
  template<typename Src_F, sfinae::require_input_ftor<Src_F> = 0>
  R operator<<(Src_F &&src) {
    return derived().read_all(FWD(src));
  }

  //! \copydoc DoubleLeftArrow_Functor
  template<typename Src_F, sfinae::require_input_ftor<Src_F> = 0>
  R operator<<(Src_F &&src) const {
    return derived().read_all(FWD(src));
  }

  //! \k2o_doc{ReadAll_Iterator}
  //! \copybrief DoubleLeftArrow_Functor
  //! \param src Input iterator to the byte sequence to read

  //! \copydoc ReadAll_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void read_all(It it) {
    derived().read_all([&]() { return *it++; });
  }

  //! \copydoc ReadAll_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void read_all(It it) const {
    derived().read_all([&]() { return *it++; });
  }

  //! \copydoc ReadAll_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void operator<<(It src) {
    read_all(src);
  }

  //! \copydoc ReadAll_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void operator<<(It src) const {
    read_all(src);
  }
};

//! \brief CRTP base class used to define immediate writing members functions
//! \details
//!   Immediate writers are able to write a byte sequence completely.
//!   Derived classes must define a 'write_all' invocable on an input functor.
template<typename D>
class immediate_writer {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  //! \k2o_doc{DoubleRightArrow_Functor}
  //! \brief Call the 'write_all' member function of derived class
  //! \param src Output functor used to write the byte sequence

  //! \copydoc DoubleRightArrow_Functor
  template<typename Dest_F, sfinae::require_output_ftor<Dest_F> = 0>
  void operator>>(Dest_F &&dest) {
    derived().write_all(FWD(dest));
  }

  //! \copydoc DoubleRightArrow_Functor
  template<typename Dest_F, sfinae::require_output_ftor<Dest_F> = 0>
  void operator>>(Dest_F &&dest) const {
    derived().write_all(FWD(dest));
  }

  //! \k2o_doc{WriteAll_Iterator}
  //! \copybrief DoubleRightArrow_Functor
  //! \param src Output iterator used to write the byte sequence

  //! \copydoc WriteAll_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void write_all(It it) {
    derived().write_all([&](upd::byte_t byte) { *it++ = byte; });
  }

  //! \copydoc WriteAll_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void write_all(It it) const {
    derived().write_all([&](upd::byte_t byte) { *it++ = byte; });
  }

  //! \copydoc WriteAll_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void operator>>(It src) {
    write_all(src);
  }

  //! \copydoc WriteAll_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void operator>>(It src) const {
    write_all(src);
  }
};

//! \brief CRTP base class used to define reading members functions
//! \details
//!   Readers are able to read a byte sequence byte per byte.
//!   Derived classes must define a 'read' invocable on an input functor.
template<typename D, typename R>
class reader : public immediate_reader<D, R> {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  //! \k2o_doc{Read_Registry}
  //! \brief Call the 'read' member function of the derived class
  //! \param reg Register to read a byte from

  //! \copydoc Read_Registry
  void read(const volatile upd::byte_t &reg) {
    derived().read([&]() { return reg; });
  }

  //! \copydoc Read_Registry
  void read(const volatile upd::byte_t &reg) const {
    derived().read([&]() { return reg; });
  }

  //! \copydoc Read_Registry
  void operator<<(const volatile upd::byte_t &reg) {
    derived().read([&]() { return reg; });
  }

  //! \copydoc Read_Registry
  void operator<<(const volatile upd::byte_t &reg) const {
    derived().read([&]() { return reg; });
  }

  //! \k2o_doc{Read_Iterator}
  //! \copybrief Read_Registry
  //! \param it Input iterator to the byte to read

  //! \copydoc Read_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void read(It it) {
    derived().read([&]() { return *it; });
  }

  //! \copydoc Read_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void read(It it) const {
    derived().read([&]() { return *it; });
  }
};

//! \brief CRTP base class used to define writing members functions
//! \details
//!   Writers are able to write a byte sequence byte per byte.
//!   Derived classes must define a 'write' invocable on an output functor.
template<typename D>
class writer : public immediate_writer<D> {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  //! \k2o_doc{Write_Registry}
  //! \brief Call the 'write' member function of the derived class
  //! \param reg Register to write a byte to

  //! \copydoc Write_Registry
  void write(const volatile upd::byte_t &reg) {
    derived().write([&]() { return reg; });
  }

  //! \copydoc Write_Registry
  void write(const volatile upd::byte_t &reg) const {
    derived().write([&]() { return reg; });
  }

  //! \copydoc Write_Registry
  void operator>>(const volatile upd::byte_t &reg) {
    derived().write([&]() { return reg; });
  }

  //! \copydoc Write_Registry
  void operator>>(const volatile upd::byte_t &reg) const {
    derived().write([&]() { return reg; });
  }

  //! \k2o_doc{Write_Iterator}
  //! \copybrief Write_Registry
  //! \param it Output iterator used to write the byte

  //! \copydoc Write_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void read(It it) {
    derived().write([&]() { return *it; });
  }

  //! \copydoc Write_Iterator
  template<typename It, sfinae::require_iterator<It> = 0>
  void read(It it) const {
    derived().write([&]() { return *it; });
  }
};

} // namespace detail
} // namespace k2o

#include "undef.hpp"
