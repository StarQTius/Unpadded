//! \file
//! \brief Functor aggregates indexing

#pragma once

#include "detail/find_in_typelist.hpp"
#include "detail/value_h.hpp"

namespace k2o {
#if __cplusplus >= 201703L
//! \brief (C++17) Functors identifier aggregator and indexer
//! \details
//!   This class makes the generation of 'ikey' object far easier than indexing them manually. To do so :
//! \code
//!   k2o::keyring<f1, f2, f3, f4, ..., fn> keyring;
//!   auto ikey = keyring.get<fk>();
//! \endcode
//!   In that case, 'ikey' will be of type 'k2o::ikey<k, decltype(fk)>'.
//!   Note that the provided template parameter can be any kind of functor, and that they do not have to be defined in
//!   the same translation unit as the 'keyring' object. This allows to provide externally linked variables or
//!   functions that are only declared in the translation unit.
//! \tparam Ftors... functors to be indexed
template<auto &... Ftors>
class keyring {
  using search_list = boost::mp11::mp_list<detail::value_h<decltype(Ftors), Ftors>...>;

  static_assert((detail::is_callable(Ftors) && ...), "keyring only accepts callable objects as template parameters");

public:
  //! \brief Make an 'ikey' object whose template parameters are determined according to the provided function
  //! \tparam Ftor functor used to make the 'ikey' object
  //! \return 'k2o::ikey<I, S>', where 'I' is the position of 'Ftor' in 'Ftors' and 'S' its signature
  template<auto &Ftor>
  constexpr auto get() const {
    using ftor_t = decltype(Ftor);
    using value_t = detail::value_h<ftor_t, Ftor>;
    return ikey<detail::find_in_typelist<value_t, search_list>(), detail::signature_t<ftor_t>>{};
  }
};
#endif // __cplusplus >= 201703L

} // namespace k2o
