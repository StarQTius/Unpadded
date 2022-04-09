//! \file

#pragma once

#include <cstddef>
#include <type_traits>

#include "../typelist.hpp"
#include "detector.hpp"

#include "../def.hpp"

namespace k2o {
namespace detail {

//! \name
//! \brief Allows to hold a function signature
//!
//!  The type member `type` is an alias for the type of a function of the given signature. The type member `return_type`
//!  is an alias for the return type of a function of the given signature.
//! @{

template<typename F>
struct signature;
template<typename R, typename... Args>
struct signature<R(Args...)> {
  using type = R(Args...);
  using return_type = R;
};

//! @}

template<typename F>
struct examine_invocable_impl : examine_invocable_impl<decltype(&F::operator())> {};
template<typename R, typename... Args>
struct examine_invocable_impl<R (*)(Args...)> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...)> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) const> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) volatile> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) const volatile> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) &> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) const &> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) volatile &> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) const volatile &> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) &&> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) const &&> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) volatile &&> : signature<R(Args...)> {};
template<typename R, typename C, typename... Args>
struct examine_invocable_impl<R (C::*)(Args...) const volatile &&> : signature<R(Args...)> {};

//! \brief Gets information on the signature of an invocable object
//!
//! Given `F` a function type or an object type which defines an unoverloaded `operator()` member,
//! `examine_invocable<F>` is an alias for the `signature` template instance which holds the signature of `F` or the
//! signature of its `operator()` member.
template<typename F>
using examine_invocable = examine_invocable_impl<typename std::decay<F>::type>;

//! \brief  Alias for 'typename examine_invocable<F>::type'
template<typename F>
using signature_t = typename examine_invocable<F>::type;

//! \brief Alias for 'typename examine_invocable<F>::return_type'
template<typename F>
using return_t = typename examine_invocable<F>::return_type;

K2O_DETAIL_MAKE_DETECTOR(is_invocable_impl, PACK(typename F), PACK(typename = examine_invocable<F>))

//! \brief Indicates whether `F` is an invocable type
template<typename F>
struct is_invocable : decltype(is_invocable_impl<F>(0)) {};

//! \name
//! \brief Sums the sizes of the parameters of `F`
//! @{

template<typename F>
struct parameters_size : parameters_size<signature_t<F>> {};
template<typename R, typename... Args>
struct parameters_size<R(Args...)> : sum<boost::mp11::mp_list<std::integral_constant<std::size_t, sizeof(Args)>...>> {};

//! @}

//! \brief Gets the size of the return type of `F`
template<typename F>
struct return_type_size : std::integral_constant<std::size_t, sizeof(return_t<F>)> {};

K2O_DETAIL_MAKE_DETECTOR(
    has_signature_impl,
    PACK(typename F, typename R, typename... Args),
    PACK(typename = typename std::enable_if<
             std::is_convertible<decltype(std::declval<F>()(std::declval<Args>()...)), R>::value>::type))

//! \name
//! \brief Indicates whether the instances of `F` can be invoked as they were functions of type `R(Args...)`
//! @{

template<typename, typename>
struct has_signature : std::false_type {};
template<typename F, typename R, typename... Args>
struct has_signature<F, R(Args...)> : decltype(has_signature_impl<F, R, Args...>(0)) {};

//! @}

} // namespace detail
} // namespace k2o

#include "../undef.hpp" // IWYU pragma: keep
