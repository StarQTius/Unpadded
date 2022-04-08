#pragma once

//! \file
//! \brief Functor introspection

#include <cstddef>
#include <type_traits>

#include <boost/mp11.hpp> // IWYU pragma: keep
#include <boost/type_traits/decay.hpp>

#include "type_traits/require.hpp"
#include "typelist.hpp"

// IWYU pragma: no_include "boost/mp11/detail/mp_list.hpp"

namespace k2o {
namespace detail {

//! \brief Allows to hold a function signature
//! \details
//!   The type member 'type' is an alias for the type of a function of the given signature. The type member
//!   'return_type' is an alias for the return type of a function of the given signature.
//! \tparam F Signature
template<typename F>
struct signature;
#ifndef DOXYGEN
template<typename R, typename... Args>
struct signature<R(Args...)> {
  using type = R(Args...);
  using return_type = R;
};
#endif

//! \brief Primary template of examine_functor
//! \details
//!   If no partial specialization matches, this definition treats the given type as a functor and tries to apply
//!   examine_functor to the operator() member function.
//! \tparam F Type of the functor
template<typename F>
struct examine_functor_impl : examine_functor_impl<decltype(&F::operator())> {};

//! \brief Partial specialization dedicated to non-member function types
//! \details
//!   This class inherits from signature<R(Args...)>.
//! \tparam R Return type of the function type
//! \tparam Args... Types of the arguments of the function type
template<typename R, typename... Args>
struct examine_functor_impl<R (*)(Args...)> : signature<R(Args...)> {};

//! \brief Partial specialization dedicated to member function types
//! \details
//!   These classes inherit from signature<R(Args...)>.
//!   Additional partial specialization are defined (but does not appear in the documentation) to deal with cv-qualified
//!   or ref-qualified member function types.
//! \tparam R Return type of the member function type
//! \tparam C Class which the member function type belongs to
//! \tparam Args... Types of the arguments of the member function type
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

template<typename F, detail::require_is_function<F> = 0>
constexpr bool is_callable_impl(int) {
  return true;
}
template<typename F, typename = decltype(&boost::decay_t<F>::operator())>
constexpr bool is_callable_impl(int) {
  return true;
}
template<typename>
constexpr bool is_callable_impl(...) {
  return false;
}

//! \brief Check if the provided object is callable
template<typename F>
constexpr bool is_callable(F &&) {
  return is_callable_impl<F>(0);
}
template<typename F>
constexpr bool is_callable() {
  return is_callable_impl<F>(0);
}

template<typename F>
struct parameters_size : parameters_size<signature_t<F>> {};

template<typename R, typename... Args>
struct parameters_size<R(Args...)> : sum<boost::mp11::mp_list<std::integral_constant<std::size_t, sizeof(Args)>...>> {};

template<typename F>
struct return_type_size : std::integral_constant<std::size_t, sizeof(return_t<F>)> {};

} // namespace detail
} // namespace k2o
