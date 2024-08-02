//! \file

#pragma once

#include <type_traits>

#include "detail/type_traits/remove_cv_ref.hpp"
#include "detail/type_traits/signature.hpp"
#include "detail/type_traits/smallest.hpp"
#include "detail/type_traits/typelist.hpp"
#include "format.hpp"
#include "key.hpp"
#include "typelist.hpp"
#include "unevaluated.hpp" // IWYU pragma: keep

// IWYU pragma: no_forward_declare unevaluated

namespace upd {

template<endianess, signed_mode, typename...>
class keyring {};

//! \brief Holds a set of callbacks and index them at compile-time
//!
//! A keyring introduces a RPC convention between the caller and the callee. The caller must have access to the
//! declarations of the callbacks given to the keyring so it knows what are the parameters to send for each action and
//! what type of value it has to except in return. However, only the callee must have access to the callback
//! definitions, in order to execute any action request from the caller. Each callback is associated with a key in the
//! keyring. For each callback, its key can be used by the caller to build a packet to send to the callee or interpret
//! the value received from the callee. Keyrings also holds the endianess and signed number representation of the data
//! in the packets. \see \ref<key> key for further information on how packets are generated.
//!
//! \tparam Endianess, Signed_Mode Serialization parameters
//! \tparam Hs Unevaluated references to callbacks available for calling
#ifdef DOXYGEN
template<endianess Endianess, signed_mode Signed_Mode, typename... Hs>
class keyring
#else  // DOXYGEN
template<endianess Endianess, signed_mode Signed_Mode, typename... Fs, Fs... Functions>
class keyring<Endianess, Signed_Mode, unevaluated<Fs, Functions>...>
#endif // DOXYGEN
{
public:
  //! \brief Typelist containing unevaluated references to the callbacks
  using flist_t = upd::flist_t<unevaluated<Fs, Functions>...>;

  //! \brief Typelist containing the signatures of the callbacks
  using signatures_t = typelist_t<detail::signature_t<Fs>...>;

  //! \brief Type of the index prepended to the payload when sending a packet to the callee
  using index_t = detail::smallest_unsigned_t<sizeof...(Fs)>;

  //! \brief Type of the key associated to an unevaluated reference to a callback
  //! \tparam H Unevaluated reference to a callback managed by the keyring
  template<typename H>
  using key_t = key<index_t,
                    detail::find<flist_t, H>::value,
                    typename std::remove_pointer<typename H::type>::type,
                    Endianess,
                    Signed_Mode>;

  //! \brief Number of managed callbacks
  constexpr static index_t size = sizeof...(Fs);

  //! \brief Endianess of the data in the packets built by the keys
  constexpr static auto endianess = Endianess;

  //! \brief Signed number representation of the data in the packets built by the keys
  constexpr static auto signed_mode = Signed_Mode;

#if __cplusplus >= 201703L
  constexpr keyring() = default;

  //! \brief (C++17) Create a keyring managing the given callbacks with the provided serialization parameters
  constexpr explicit keyring(upd::flist_t<unevaluated<Fs, Functions>...>,
                             endianess_h<Endianess>,
                             signed_mode_h<Signed_Mode>) {}

  constexpr explicit keyring(upd::flist_t<unevaluated<Fs, Functions>...> fl) {}
#endif // __cplusplus >= 201703L

  //! \brief Make a key associated with the given unevaluated reference to a callback
  //! \tparam H Unevaluated reference to a callback managed by the keyring
  template<typename H>
  constexpr key_t<H> get(H) const {
    return {};
  }

#if __cplusplus >= 201703L
  //! \brief Make a key associated with the given unevaluated reference to a callback
  //! \tparam Ftor One of the callbacks managed by the keyring
  template<auto &Ftor>
  constexpr auto get() const {
    return get(unevaluated<detail::remove_cv_ref_t<decltype(Ftor)> *, &Ftor>{});
  }
#endif // __cplusplus >= 201703L
};

#if __cplusplus >= 201703L
template<typename... Hs, endianess Endianess, signed_mode Signed_Mode>
keyring(flist_t<Hs...>, endianess_h<Endianess>, signed_mode_h<Signed_Mode>) -> keyring<Endianess, Signed_Mode, Hs...>;

template<typename... Hs>
keyring(flist_t<Hs...>) -> keyring<endianess::LITTLE, signed_mode::TWOS_COMPLEMENT, Hs...>;
#endif // __cplusplus >= 201703L

//! \brief Make a keyring objects
//! \related keyring
template<endianess Endianess, signed_mode Signed_Mode, typename... Hs>
constexpr keyring<Endianess, Signed_Mode, Hs...>
make_keyring(flist_t<Hs...>, endianess_h<Endianess>, signed_mode_h<Signed_Mode>) {
  return {};
}

} // namespace upd
