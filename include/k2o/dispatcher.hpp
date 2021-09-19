//! \file
//! \brief Key index resolution

#pragma once

#include "detail/sfinae.hpp"

#include "keyring11.hpp"
#include "order.hpp"
#include "status.hpp"

namespace k2o {

//!
template<size_t N>
class dispatcher {
public:
  template<typename... Fs, Fs... Ftors>
  explicit dispatcher(keyring11<detail::unevaluated_value_h<Fs, Ftors>...>) : orders{order{Ftors}...} {}

  template<typename Src_F, typename Dest_F>
  status operator()(Src_F &&fetch_byte, Dest_F &&insert_byte) {
    using namespace upd;

    tuple<endianess::BUILTIN, signed_mode::BUILTIN, uint16_t> order_index;
    for (auto &byte : order_index)
      byte = K2O_FWD(fetch_byte)();

    return orders[order_index.get<0>()](K2O_FWD(fetch_byte), K2O_FWD(insert_byte));
  }

private:
  order orders[N];
};

template<typename... Fs, Fs... Ftors>
dispatcher<sizeof...(Fs)> make_dispatcher(keyring11<detail::unevaluated_value_h<Fs, Ftors>...> input_keyring) {
  return dispatcher<sizeof...(Fs)>{input_keyring};
}

} // namespace k2o
