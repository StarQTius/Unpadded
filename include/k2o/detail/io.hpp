//! \file
//! \brief Input / output utilities

#pragma once

#include <upd/type.hpp>

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
  //! \name Immediate reading functions
  //! \brief Call the 'read_all' member function of the derived class
  //! \details
  //!   These functions may be invoked on hardware registers and input functors and iterators
  //! @{

  template<typename Src_F, sfinae::require_input_ftor<Src_F> = 0>
  R operator<<(Src_F &&src) {
    return derived().read_all(FWD(src));
  }

  template<typename Src_F, sfinae::require_input_ftor<Src_F> = 0>
  R operator<<(Src_F &&src) const {
    return derived().read_all(FWD(src));
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  R read_all(It it) {
    return derived().read_all([&]() { return *it++; });
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  R read_all(It it) const {
    return derived().read_all([&]() { return *it++; });
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  R operator<<(It src) {
    return read_all(src);
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
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

//! \brief CRTP base class used to define immediate writing members functions
//! \details
//!   Immediate writers are able to write a byte sequence completely.
//!   Derived classes must define a 'write_all' invocable on an input functor.
template<typename D>
class immediate_writer {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  //! \name Immediate writing functions
  //! \brief Call the 'write_all' member function of the derived class
  //! \details
  //!   These functions may be invoked on hardware registers and output functors and iterators
  //! @{

  template<typename Dest_F, sfinae::require_output_ftor<Dest_F> = 0>
  void operator>>(Dest_F &&dest) {
    derived().write_all(FWD(dest));
  }

  template<typename Dest_F, sfinae::require_output_ftor<Dest_F> = 0>
  void operator>>(Dest_F &&dest) const {
    derived().write_all(FWD(dest));
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  void write_all(It it) {
    derived().write_all([&](upd::byte_t byte) { *it++ = byte; });
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  void write_all(It it) const {
    derived().write_all([&](upd::byte_t byte) { *it++ = byte; });
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  void operator>>(It src) {
    write_all(src);
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  void operator>>(It src) const {
    write_all(src);
  }

  //! @}
};

//! \class ImmediateWriter_CRTP
//!
//! The following member functions are also defined through CRTP.
//! \code
//! void operator>>(Dest &&);
//! void operator>>(Dest &&) const;
//! void write_all(It);
//! void write_all(It) const;
//! void operator>>(It);
//! void operator>>(It) const;
//! \endcode
//! All these functions work the same way as `write_all(Dest &&)`.
//!
//! \tparam Dest Output functor type
//! \tparam It Output iterator type

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

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  void read(It it) {
    derived().read([&]() { return *it; });
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
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

  void write(const volatile upd::byte_t &reg) {
    derived().write([&]() { return reg; });
  }

  void write(const volatile upd::byte_t &reg) const {
    derived().write([&]() { return reg; });
  }

  using immediate_writer<D>::operator>>;

  void operator>>(const volatile upd::byte_t &reg) {
    derived().write([&]() { return reg; });
  }

  void operator>>(const volatile upd::byte_t &reg) const {
    derived().write([&]() { return reg; });
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  void write(It it) {
    derived().write([&]() { return *it; });
  }

  template<typename It, sfinae::require_byte_iterator<It> = 0>
  void write(It it) const {
    derived().write([&]() { return *it; });
  }

  // @}
};

//! \class Writer_CRTP
//!
//! The following member functions are also defined through CRTP.
//! \code
//! void write(const volatile upd::byte_t &);
//! void write(const volatile upd::byte_t &) const;
//! void operator>>(const volatile upd::byte_t &);
//! void operator>>(const volatile upd::byte_t &) const;
//! void write(It);
//! void write(It) const;
//! \endcode
//! All these functions work the same way as `write(Dest &&)`.
//!
//! \tparam It Output iterator type

//! \brief CRTP base class used to define class whose instances process input and yield output immediately
//! \details
//!   Immediate process can be invoked on a byte input and a byte output.
//!   Derived classes must be invocable on an input functor an output functor.
template<typename D, typename R>
class immediate_process {
  D &derived() { return reinterpret_cast<D &>(*this); }
  const D &derived() const { return reinterpret_cast<const D &>(*this); }

public:
  //! \brief Normalize the parameters and invoke the derived instance on them
  //! \param input Byte input to process
  //! \param output Byte output to write to
  //! \return the result of the invocation of the derived instance on the normalized parameters
  template<typename Input,
           typename Output,
           sfinae::require<!sfinae::has_signature<Input, upd::byte_t()>::value ||
                           !sfinae::has_signature<Output, void(upd::byte_t)>::value> = 0>
  R operator()(Input &&input, Output &&output) {
    return derived()(normalize(FWD(input), reader_tag_t{}), normalize(FWD(output), writer_tag_t{}));
  }

  //! \copydoc operator()
  template<typename Input,
           typename Output,
           sfinae::require<!sfinae::has_signature<Input, upd::byte_t()>::value ||
                           !sfinae::has_signature<Output, void(upd::byte_t)>::value> = 0>
  R operator()(Input &&input, Output &&output) const {
    return derived()(normalize(FWD(input), reader_tag_t{}), normalize(FWD(output), writer_tag_t{}));
  }

private:
  struct reader_tag_t {};
  struct writer_tag_t {};

  //! \brief Iterator wrapper behaving like an input functor
  template<typename It>
  struct reader_iterator {
    upd::byte_t operator()() { return *it++; }
    It it;
  };

  //! \brief Iterator wrapper behaving like an output functor
  template<typename It>
  struct writer_iterator {
    void operator()(upd::byte_t byte) { *it++ = byte; }
    It it;
  };

  //! Behave as an identity function
  template<typename F>
  F &&normalize(F &&ftor, ...) {
    return FWD(ftor);
  }

  //! Behave as an identity function
  template<typename F>
  F &&normalize(F &&ftor, ...) const {
    return FWD(ftor);
  }

  //! Wrap the iterator to get an input functor
  template<typename It, sfinae::require_byte_iterator<It> = 0>
  reader_iterator<It> normalize(It it, reader_tag_t) {
    return {it};
  }

  //! Wrap the iterator to get an input functor
  template<typename It, sfinae::require_byte_iterator<It> = 0>
  reader_iterator<It> normalize(It it, reader_tag_t) const {
    return {it};
  }

  //! Wrap the iterator to get an output functor
  template<typename It, sfinae::require_byte_iterator<It> = 0>
  writer_iterator<It> normalize(It it, writer_tag_t) {
    return {it};
  }

  //! Wrap the iterator to get an output functor
  template<typename It, sfinae::require_byte_iterator<It> = 0>
  writer_iterator<It> normalize(It it, writer_tag_t) const {
    return {it};
  }
};

//! \class ImmediateProcess_CRTP
//!
//! The following member functions are also defined through CRTP.
//! \code
//! auto operator()(Input &&, Output &&);
//! auto operator()(Input &&, Output &&) const;
//! \endcode
//! All these functions work the same way as `operator()(Src &&, Dest &&)`.
//!
//! \tparam Input An input functor type or an input iterator type
//! \tparam Output An output functor type or an output iterator type

} // namespace detail
} // namespace k2o

#include "undef.hpp" // IWYU pragma: keep
