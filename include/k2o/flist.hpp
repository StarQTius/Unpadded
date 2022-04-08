#pragma once

#include "detail/unevaluated.hpp" // IWYU pragma: keep

namespace k2o {

template<typename...>
struct flist_t;

template<typename... Fs, Fs... Functions>
struct flist_t<detail::unevaluated<Fs, Functions>...> {};

#if __cplusplus >= 201703L
template<auto &...Functions>
flist_t<detail::unevaluated<decltype(Functions), Functions>...> flist;
#endif // __cplusplus >= 201703L

} // namespace k2o
