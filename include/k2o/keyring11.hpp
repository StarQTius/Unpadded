//! \file
//! \brief Function aggregates indexing for C++11

#pragma once

#include "detail/find_in_typelist.hpp"
#include "detail/signature.hpp"
#include "detail/value_h.hpp"

#include "ikey.hpp"

#include "detail/def.hpp"

namespace k2o {

template<upd::endianess, upd::signed_mode, typename... Hs>
class keyring11;

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs... Functions>
class keyring11<Endianess, Signed_Mode, detail::unevaluated_value_h<Fs, Functions>...> {
  using search_list = boost::mp11::mp_list<detail::unevaluated_value_h<Fs, Functions>...>;

  template<typename H>
  using key_t =
      ikey<detail::find_in_typelist<H, search_list>(), detail::signature_t<typename H::type>, Endianess, Signed_Mode>;

  STATIC_ASSERT((boost::conjunction<boost::integral_constant<bool, detail::is_callable<Fs>()>...>::value),
                "'keyring11' only accepts callable objects as template parameters");

public:
  template<typename H>
  constexpr key_t<H> get() const {
    return {};
  }
};

} // namespace k2o

#include "detail/undef.hpp"
