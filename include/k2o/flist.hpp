#pragma once

#include "detail/value_h.hpp" // IWYU pragma: keep

namespace k2o {

template<typename...>
struct flist11_t;

template<typename... Fs, Fs... Functions>
struct flist11_t<detail::unevaluated_value_h<Fs, Functions>...> {};

#if __cplusplus >= 201703L
template<auto &...Functions>
struct flist_t : flist11_t<detail::unevaluated_value_h<decltype(Functions), Functions>...> {};

template<auto &...Functions>
flist_t<Functions...> flist;
#endif // __cplusplus >= 201703L

} // namespace k2o
