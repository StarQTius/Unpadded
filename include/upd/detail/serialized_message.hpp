//! \file

#pragma once

#include "../format.hpp"
#include "../tuple.hpp"

#include "io/immediate_writer.hpp"
#include "type_traits/require.hpp"

#include "def.hpp"

namespace upd {
namespace detail {

//! \brief Simple wrapper around 'tuple' whose content can be forwarded to a functor as a byte sequence
//! \details
//!   The content can be forwarded with the 'operator>>' function member. 'detail::serialized_message' object
//!   cannot be copied from to avoid unintentional copy.
template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
struct serialized_message : detail::immediate_writer<serialized_message<Endianess, Signed_Mode, Ts...>> {
  //! \brief Store the payload
  serialized_message(const Ts &...values) : content{values...} {}

  serialized_message(const serialized_message &) = delete;
  serialized_message(serialized_message &&) = default;

  serialized_message &operator=(const serialized_message &) = delete;
  serialized_message &operator=(serialized_message &&) = default;

  using detail::immediate_writer<serialized_message<Endianess, Signed_Mode, Ts...>>::write_all;

  //! \brief Completely output the payload represented by the key
  template<typename Dest_F, REQUIREMENT(output_invocable, Dest_F)>
  void write_all(Dest_F &&insert_byte) const {
    for (auto byte : content)
      insert_byte(byte);
  }

  tuple<Endianess, Signed_Mode, Ts...> content;
};

} // namespace detail
} // namespace upd

#include "undef.hpp" // IWYU pragma: keep
