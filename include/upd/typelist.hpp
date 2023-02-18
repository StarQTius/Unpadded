//! \file

#pragma once

#include <type_traits>

#include "detail/static_error.hpp"
#include "detail/type_traits/conjunction.hpp"
#include "detail/type_traits/remove_cv_ref.hpp"
#include "detail/type_traits/signature.hpp" // IWYU pragma: keep
#include "detail/type_traits/typelist.hpp"
#include "unevaluated.hpp" // IWYU pragma: keep

// IWYU pragma: no_forward_declare detail::is_invocable

namespace upd {

//! \brief Utility class used to holds a list of template argument
//! \tparam Ts List of template argument to hold
template<typename... Ts>
struct typelist_t : detail::tlist_t<Ts...> {};

template<typename...>
struct flist_t {};

//! \brief Typelist holding references to objects with static storage duration
//! \tparam Functions Invocable objects bound by reference
template<typename... Fs, Fs... Functions>
struct flist_t<unevaluated<Fs, Functions>...> : typelist_t<unevaluated<Fs, Functions>...> {
  static_assert(detail::conjunction<detail::is_invocable<decltype(*Functions)>...>::value,
                UPD_ERROR_NOT_ALL_INVOCABLE(Functions));
  static_assert(detail::conjunction<std::is_lvalue_reference<decltype(*Functions)>...>::value,
                UPD_ERROR_NOT_ALL_LVALUE(Functions));
};

#if __cplusplus >= 201703L

//! \copydoc typelist_t
template<typename... Ts>
constexpr typelist_t<Ts...> typelist;

//! \brief Typelist holding references to objects with static storage duration
//! \tparam Functions Invocable objects bound by reference
template<auto &...Functions>
constexpr flist_t<unevaluated<detail::remove_cv_ref_t<decltype(Functions)> *, &Functions>...> flist;
#endif // __cplusplus >= 201703L

//! \brief Make a typelist of references to invocable objects
//! \related flist_t
template<typename... Fs, Fs... Functions>
constexpr flist_t<unevaluated<Fs, Functions>...> make_flist(unevaluated<Fs, Functions>...) {
  return {};
};

} // namespace upd
