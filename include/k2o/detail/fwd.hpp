#pragma once

#include <boost/type_traits.hpp>

namespace k2o {
namespace detail {

template<typename T>
boost::remove_reference_t<T> &&move(T &&x) {
  return static_cast<boost::remove_reference_t<T> &&>(x);
}

} // namespace detail
} // namespace k2o

#define FWD(x) static_cast<decltype(x) &&>(x)
