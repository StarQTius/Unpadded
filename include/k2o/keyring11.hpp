//! \file
//! \brief Function aggregates indexing for C++11

#pragma once

#include "detail/find_in_typelist.hpp"
#include "detail/value_h.hpp"

namespace k2o {

template<typename... Hs>
class keyring11;

template<typename... Fs, Fs... Functions>
class keyring11<detail::value_h<Fs, Functions>...> {
  using search_list = boost::mp11::mp_list<detail::value_h<Fs, Functions>...>;

public:
  template<typename H>
  constexpr ikey<detail::find_in_typelist<H, search_list>(), detail::signature_t<decltype(H::value)>> get() const {
    return {};
  }
};

} // namespace k2o
