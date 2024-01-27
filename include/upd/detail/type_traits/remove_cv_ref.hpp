//! \file

#include <type_traits>

namespace upd::detail {

//! \brief Remove any cv-qualifier or ref-qualifier
template<typename T>
using remove_cv_ref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

//! \brief Same behavior as `std::decay_t`
template<typename T>
using decay_t = typename std::decay<T>::type;

} // namespace upd::detail
