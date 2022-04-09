#pragma once

#include "unevaluated.hpp" // IWYU pragma: keep

namespace k2o {

template<typename...>
struct flist_t;

template<typename... Fs, Fs... Functions>
struct flist_t<unevaluated<Fs, Functions>...> {};

#if __cplusplus >= 201703L
template<auto &...Functions>
flist_t<unevaluated<decltype(Functions), Functions>...> flist;
#endif // __cplusplus >= 201703L

template<typename... Fs, Fs... Functions>
constexpr flist_t<unevaluated<Fs, Functions>...> make_flist(unevaluated<Fs, Functions>...) {
  return {};
};

} // namespace k2o
