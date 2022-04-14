#pragma once

#include "unevaluated.hpp" // IWYU pragma: keep

namespace k2o {

template<typename...>
struct flist_t;

//! \brief Typelist holding references to objects with static storage duration
//! \tparam Functions Invocable objects bound by reference
template<typename... Fs, Fs... Functions>
struct flist_t<unevaluated<Fs, Functions>...> {};

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
