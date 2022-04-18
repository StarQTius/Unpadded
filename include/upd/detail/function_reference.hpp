//! \file

#pragma once

#include "type_traits/signature.hpp"

namespace upd {
namespace detail {

template<typename>
struct abstract_function;

//! \brief Abstract class defining a functor interface
//! \tparam R return value of the functor
//! \tparam Args... types of the functor parameters
template<typename R, typename... Args>
struct abstract_function<R(Args...)> {
  virtual ~abstract_function() = default;
  virtual R operator()(Args...) = 0;
};

template<typename...>
class function_reference_impl;

//! \brief Implementation of 'function_reference'
//! \details
//!   This class is base of 'function_reference'. It could be used as it is, though it would impraticable as the
//!   functor type, its return and parameter types must be provided explicitely. The 'function_reference' is just here
//!   to deduce the return and parameter types from the functor type and derive from the according template instance of
//!   'function_reference_impl'.
//! \tparam F type of the held functor
//! \tparam R return type of the held functor
//! \tparam Args... parameter types of the held functor
template<typename F, typename R, typename... Args>
class function_reference_impl<F, R(Args...)> : public abstract_function<R(Args...)> {
public:
  explicit function_reference_impl(F &ftor) : m_ftor{ftor} {}

  R operator()(Args... args) final { return m_ftor(static_cast<Args &&>(args)...); }

private:
  F &m_ftor;
};

//! \brief Wrap a reference to a functor and allow calling it through the interface defined by 'abstract_function'
//! \details
//!   This class allow type erasure without dynamic allocation. However, the wrapped functor is held by reference and
//!   is not copied (as it would be with 'std::function').
//! \tparam F type of the functor wrapped by reference
template<typename F>
class function_reference : public function_reference_impl<F, signature_t<F>> {
public:
  using function_reference_impl<F, signature_t<F>>::function_reference_impl;
};

//! \brief Wrap a functor by reference into a 'function_reference' object
//! \param ftor wrapped functor
//! \return a 'function_reference' object wrapping the provided functor
template<typename F>
function_reference<F> make_function_reference(F &ftor) {
  return function_reference<F>(ftor);
}

} // namespace detail
} // namespace upd
