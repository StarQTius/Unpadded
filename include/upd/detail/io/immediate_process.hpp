//! \file

#pragma once

#include <upd/type.hpp>

#include "../type_traits/is_byte_iterator.hpp"
#include "../type_traits/require.hpp"

#include "../def.hpp"

namespace k2o {
namespace detail {

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
           detail::require<detail::is_byte_iterator<Input>::value || detail::is_byte_iterator<Output>::value> = 0>
  R operator()(Input &&input, Output &&output) {
    return derived()(normalize(FWD(input), reader_tag_t{}), normalize(FWD(output), writer_tag_t{}));
  }

  //! \copydoc operator()
  template<typename Input,
           typename Output,
           detail::require<detail::is_byte_iterator<Input>::value || detail::is_byte_iterator<Output>::value> = 0>
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
  template<typename It, detail::require_byte_iterator<It> = 0>
  reader_iterator<It> normalize(It it, reader_tag_t) {
    return {it};
  }

  //! Wrap the iterator to get an input functor
  template<typename It, detail::require_byte_iterator<It> = 0>
  reader_iterator<It> normalize(It it, reader_tag_t) const {
    return {it};
  }

  //! Wrap the iterator to get an output functor
  template<typename It, detail::require_byte_iterator<It> = 0>
  writer_iterator<It> normalize(It it, writer_tag_t) {
    return {it};
  }

  //! Wrap the iterator to get an output functor
  template<typename It, detail::require_byte_iterator<It> = 0>
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

#include "../undef.hpp" // IWYU pragma: keep
