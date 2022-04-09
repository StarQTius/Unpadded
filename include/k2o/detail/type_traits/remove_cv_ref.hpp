//! \file

#include <type_traits>

namespace k2o {
namespace detail {

//! \brief Remove any cv-qualifier or ref-qualifier
template<typename T>
using remove_cv_ref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

} // namespace detail
} // namespace k2o
