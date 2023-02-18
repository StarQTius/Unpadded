//! \file

#pragma once

#include "../../type.hpp"
#include "../../upd.hpp"
#include "../type_traits/iterator_category.hpp"
#include "../type_traits/no_derived_member_shadowing.hpp"
#include "../type_traits/remove_cv_ref.hpp"
#include "../type_traits/require.hpp"

namespace upd {
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
  template<typename Input,
           typename Output,
           UPD_REQUIRE(is_input_byte_iterator<decay_t<Input>>::value ||
                       is_output_byte_iterator<decay_t<Output>>::value)>
  R operator()(Input &&input, Output &&output, no_derived_member_shadowing = 0) {
    return derived()(normalize(UPD_FWD(input), reader_tag_t{}), normalize(UPD_FWD(output), writer_tag_t{}));
  }

  template<typename Input,
           typename Output,
           UPD_REQUIRE(is_input_byte_iterator<decay_t<Input>>::value ||
                       is_output_byte_iterator<decay_t<Output>>::value)>
  R operator()(Input &&input, Output &&output, no_derived_member_shadowing = 0) const {
    return derived()(normalize(UPD_FWD(input), reader_tag_t{}), normalize(UPD_FWD(output), writer_tag_t{}));
  }

private:
  struct reader_tag_t {};
  struct writer_tag_t {};

  //! \brief Iterator wrapper behaving like an input functor
  template<typename It>
  struct reader_iterator {
    byte_t operator()() { return *it++; }
    It it;
  };

  //! \brief Iterator wrapper behaving like an output functor
  template<typename It>
  struct writer_iterator {
    void operator()(byte_t byte) { *it++ = byte; }
    It it;
  };

  //! Behave as an identity function
  template<typename F>
  F &&normalize(F &&ftor, ...) {
    return UPD_FWD(ftor);
  }

  //! Behave as an identity function
  template<typename F>
  F &&normalize(F &&ftor, ...) const {
    return UPD_FWD(ftor);
  }

  //! Wrap the iterator to get an input functor
  template<typename It, UPD_REQUIREMENT(input_byte_iterator, It)>
  reader_iterator<It> normalize(It it, reader_tag_t) {
    return {it};
  }

  //! Wrap the iterator to get an input functor
  template<typename It, UPD_REQUIREMENT(input_byte_iterator, It)>
  reader_iterator<It> normalize(It it, reader_tag_t) const {
    return {it};
  }

  //! Wrap the iterator to get an output functor
  template<typename It, UPD_REQUIREMENT(output_byte_iterator, It)>
  writer_iterator<It> normalize(It it, writer_tag_t) {
    return {it};
  }

  //! Wrap the iterator to get an output functor
  template<typename It, UPD_REQUIREMENT(output_byte_iterator, It)>
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
//! \tparam Input Input byte stream type of any kind
//! \tparam Output Output byte stream type of any kind

} // namespace detail
} // namespace upd
