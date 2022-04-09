//! \file
//! \brief Function aggregates indexing for C++11

#pragma once

#include <boost/mp11.hpp> // IWYU pragma: keep
#include <upd/format.hpp>

// IWYU pragma: no_include "boost/mp11/detail/mp_list.hpp"

#include "detail/type_traits/conjunction.hpp"
#include "detail/type_traits/signature.hpp"
#include "detail/type_traits/smallest.hpp"
#include "detail/type_traits/typelist.hpp"
#include "detail/unevaluated.hpp" // IWYU pragma: keep
#include "flist.hpp"
#include "key.hpp"

// IWYU pragma: no_forward_declare unevaluated

namespace k2o {

template<upd::endianess, upd::signed_mode, typename...>
class keyring;

//! \brief Holds a set of functions and index them at compile-time
//!
//!   A keyring introduces a RPC convention between the master and the slave. The master must have access to the
//!   declarations of the functions given to the keyring so it knows what are the parameters to send for each order and
//!   what type of value it has to except in return. However, only the slave must have access to the function
//!   definitions, in order to execute any order request from the master. Each function is associated with a key in the
//!   keyring. For each function, its key can be used by the master to build a packet to send to the salve or interpret
//!   the value received from the slave. Keyrings also holds the endianess and signed number representation of the data
//!   in the packets.
//! \see
//!   \mgref{key, key} for further information on how packets are generated.
//!
//! \tparam Endianess Endianess of the data in the packets built by the keys
//! \tparam Signed_Mode Signed number representation of the data in the packets built by the keys
//! \tparam Hs Unevaluated references to functions available for calling
#ifdef DOXYGEN
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Hs>
class keyring
#else  // DOXYGEN
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs... Functions>
class keyring<Endianess, Signed_Mode, detail::unevaluated<Fs, Functions>...>
#endif // DOXYGEN
{
  static_assert((detail::conjunction<detail::is_invocable<Fs>...>::value),
                "Keyrings only accepts callable objects as template parameters");

public:
  //! \brief Typelist containing unevaluated references to the functions
  using flist_t = k2o::flist_t<detail::unevaluated<Fs, Functions>...>;

  //! \brief Typelist containing the signatures of the functions
  using signatures_t = boost::mp11::mp_list<detail::signature_t<Fs>...>;

  //! \brief Type of the index prepended to the payload when sending a packet to the slave
  using index_t = detail::smallest_unsigned_t<sizeof...(Fs)>;

  //! \brief Type of the key associated to an unevaluated reference to a function
  //! \tparam H Unevaluated reference to a function managed by the keyring
  template<typename H>
  using key_t =
      key<index_t, detail::find<flist_t, H>::value, detail::signature_t<typename H::type>, Endianess, Signed_Mode>;

  //! \brief Number of managed functions
  constexpr static index_t size = sizeof...(Fs);

  //! \brief Endianess of the data in the packets built by the keys
  constexpr static auto endianess = Endianess;

  //! \brief Signed number representation of the data in the packets built by the keys
  constexpr static auto signed_mode = Signed_Mode;

#if __cplusplus >= 201703L
  constexpr keyring() = default;

  //! \brief (C++17) Create a keyring managing the given functions with the native serialization parameters
  constexpr explicit keyring(k2o::flist_t<detail::unevaluated<Fs, Functions>...>) {}

  //! \brief (C++17) Create a keyring managing the given functions with the provided serialization parameters
  constexpr explicit keyring(k2o::flist_t<detail::unevaluated<Fs, Functions>...>,
                             upd::endianess_h<Endianess>,
                             upd::signed_mode_h<Signed_Mode>) {}
#endif // __cplusplus >= 201703L

  //! \brief Make a key associated with the given unevaluated reference to a function
  //! \tparam H Unevaluated reference to a function managed by the keyring
  template<typename H>
  constexpr key_t<H> get(H) const {
    return {};
  }

#if __cplusplus >= 201703L
  //! \brief Make a key associated with the given unevaluated reference to a function
  //! \tparam Ftor One of the functors managed by the keyring
  template<auto &Ftor>
  constexpr auto get() const {
    return get(detail::unevaluated<decltype(Ftor), Ftor>{});
  }
#endif // __cplusplus >= 201703L
};

#if __cplusplus >= 201703L
template<typename... Hs>
keyring(flist_t<Hs...>) -> keyring<upd::endianess::BUILTIN, upd::signed_mode::BUILTIN, Hs...>;

template<typename... Hs, upd::endianess Endianess, upd::signed_mode Signed_Mode>
keyring(flist_t<Hs...>, upd::endianess_h<Endianess>, upd::signed_mode_h<Signed_Mode>)
    -> keyring<Endianess, Signed_Mode, Hs...>;
#endif // __cplusplus >= 201703L

//! \brief Make a keyring objects
//! \related keyring
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Hs>
constexpr keyring<Endianess, Signed_Mode, Hs...>
make_keyring(flist_t<Hs...>, upd::endianess_h<Endianess>, upd::signed_mode_h<Signed_Mode>) {
  return {};
}

//! \copybrief make_keyring
//! \related keyring
template<typename... Hs>
constexpr keyring<upd::endianess::BUILTIN, upd::signed_mode::BUILTIN, Hs...> make_keyring(flist_t<Hs...>) {
  return {};
}

} // namespace k2o
