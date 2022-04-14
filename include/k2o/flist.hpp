#pragma once

#include <type_traits>

#include "detail/static_error.hpp"
#include "detail/type_traits/conjunction.hpp"
#include "detail/type_traits/signature.hpp" // IWYU pragma: keep
#include "unevaluated.hpp"                  // IWYU pragma: keep

// IWYU pragma: no_forward_declare k2o::detail::is_invocable

namespace k2o {

template<typename...>
struct flist_t;

//! \brief Typelist holding references to objects with static storage duration
//! \tparam Functions Invocable objects bound by reference
template<typename... Fs, Fs... Functions>
struct flist_t<unevaluated<Fs, Functions>...> {
  static_assert(detail::conjunction<detail::is_invocable<Fs>...>::value, K2O_ERROR_NOT_ALL_INVOCABLE(Functions));
  static_assert(detail::conjunction<std::is_lvalue_reference<Fs>...>::value, K2O_ERROR_NOT_ALL_LVALUE(Functions));
};

#if __cplusplus >= 201703L
//! \copydoc flist_t
template<auto &...Functions>
flist_t<unevaluated<decltype(Functions), Functions>...> flist;
#endif // __cplusplus >= 201703L

//! \brief Make a typelist of references to invocable objects
//! \related flist_t
template<typename... Fs, Fs... Functions>
constexpr flist_t<unevaluated<Fs, Functions>...> make_flist(unevaluated<Fs, Functions>...) {
  return {};
};

} // namespace k2o
