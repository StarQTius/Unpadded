#pragma once

/*!
  \file
  \brief Definitions of functor introspection helper classes
*/

#include "boost/type_traits.hpp"

namespace k2o {
namespace detail {

/*!
  \brief Allows to hold a function signature
  \details
    The type member 'type' is an alias for the type of a function of the given signature.
    The type member 'return_type' is an alias for the return type of a function of the given signature.
  \tparam F Signature
*/
template<typename F>
struct signature;
#ifndef DOXYGEN
template<typename R, typename... Args>
struct signature<R(Args...)> {
  using type = R(Args...);
  using return_type = R;
};
#endif

/*!
  \brief Primary template of examine_functor
  \details
    If no partial specialization matches, this definition treats the given type as a functor and tries to apply
    examine_functor to the operator() member function.
  \tparam F Type of the functor
*/
template<typename F>
struct examine_functor_impl : examine_functor_impl<decltype(&F::operator())> {};

/*!
  \brief Partial specialization dedicated to non-member function types
  \details
    This class inherits from signature<R(Args...)>.
  \tparam R Return type of the function type
  \tparam Args... Types of the arguments of the function type
*/
template<typename R, typename... Args>
struct examine_functor_impl<R (*)(Args...)> : signature<R(Args...)> {};

/*!
  \brief Partial specialization dedicated to member function types
  \details
    These classes inherit from signature<R(Args...)>.
    Additional partial specialization are defined (but does not appear in the documentation) to deal with cv-qualified
    or ref-qualified member function types.
  \tparam R Return type of the member function type
  \tparam C Class which the member function type belongs to
  \tparam Args... Types of the arguments of the member function type
*/
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...)> : signature<R(Args...)> {};
#ifndef DOXYGEN
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) const> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) volatile> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) const volatile> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) &> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) const &> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) volatile &> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) const volatile &> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) &&> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) const &&> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) volatile &&> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_functor_impl<R (C::*)(Args...) const volatile &&> : signature<R(Args...)> {};
#endif

//! \brief  Alias for 'typename examine_functor_impl<boost::decay_t<F>>'
template<typename F>
using examine_functor = examine_functor_impl<boost::decay_t<F>>;

//! \brief  Alias for 'typename examine_functor<F>::type'
template<typename F>
using signature_t = typename examine_functor<F>::type;

//! \brief Alias for 'typename examine_functor<F>::return_type'
template<typename F>
using return_t = typename examine_functor<F>::return_type;

} // namespace detail
} // namespace k2o
