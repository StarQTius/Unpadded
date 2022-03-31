//! \file
//! \brief Function aggregates indexing for C++11

#pragma once

#include <cstdint>

#include <boost/mp11/detail/mp_list.hpp>
#include <boost/type_traits/conjunction.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <upd/format.hpp>

#include "detail/signature.hpp"
#include "detail/typelist.hpp"
#include "detail/value_h.hpp" // IWYU pragma: keep
#include "key.hpp"

// IWYU pragma: no_forward_declare boost::integral_constant
// IWYU pragma: no_forward_declare unevaluated_value_h

namespace k2o {

template<typename...>
struct flist11_t;

template<typename... Fs, Fs... Functions>
struct flist11_t<detail::unevaluated_value_h<Fs, Functions>...> {};

#if __cplusplus >= 201703L
template<auto &...Functions>
struct flist_t : flist11_t<detail::unevaluated_value_h<decltype(Functions), Functions>...> {};

template<auto &...Functions>
flist_t<Functions...> flist;
#endif

template<upd::endianess, upd::signed_mode, typename... Hs>
class keyring;

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Fs, Fs... Functions>
class keyring<Endianess, Signed_Mode, detail::unevaluated_value_h<Fs, Functions>...> {
  static_assert((boost::conjunction<boost::integral_constant<bool, detail::is_callable<Fs>()>...>::value),
                "'keyring' only accepts callable objects as template parameters");

public:
  using list = boost::mp11::mp_list<detail::unevaluated_value_h<Fs, Functions>...>;
  using signatures = boost::mp11::mp_list<Fs...>;
  template<typename H>
  using key_t = key<detail::find<list, H>::value, detail::signature_t<typename H::type>, Endianess, Signed_Mode>;

  using index_t = uint16_t;

  constexpr static auto size = sizeof...(Fs);
  constexpr static auto endianess = Endianess;
  constexpr static auto signed_mode = Signed_Mode;

  constexpr keyring() = default;
  constexpr keyring(flist11_t<detail::unevaluated_value_h<Fs, Functions>...>) {}
  constexpr keyring(flist11_t<detail::unevaluated_value_h<Fs, Functions>...>,
                    upd::endianess_h<Endianess>,
                    upd::signed_mode_h<Signed_Mode>) {}

  template<typename H>
  constexpr key_t<H> get() const {
    return {};
  }

#if __cplusplus >= 201703L
  //! \brief Make an 'key' object whose template parameters are determined according to the provided function
  //! \tparam Ftor functor used to make the 'key' object
  //! \return 'k2o::key<I, S>', where 'I' is the position of 'Ftor' in 'Ftors' and 'S' its signature
  template<auto &Ftor>
  constexpr auto get() const {
    return get<detail::unevaluated_value_h<decltype(Ftor), Ftor>>();
  }
#endif // __cplusplus >= 201703L
};

#if __cplusplus >= 201703L
template<typename... Hs>
keyring(flist11_t<Hs...>) -> keyring<upd::endianess::BUILTIN, upd::signed_mode::BUILTIN, Hs...>;

template<typename... Hs, upd::endianess Endianess, upd::signed_mode Signed_Mode>
keyring(flist11_t<Hs...>, upd::endianess_h<Endianess>, upd::signed_mode_h<Signed_Mode>)
    -> keyring<Endianess, Signed_Mode, Hs...>;
#endif // __cplusplus >= 201703L

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Hs>
constexpr keyring<Endianess, Signed_Mode, Hs...>
make_keyring(flist11_t<Hs...>, upd::endianess_h<Endianess>, upd::signed_mode_h<Signed_Mode>) {
  return {};
}

template<typename... Hs>
constexpr keyring<upd::endianess::BUILTIN, upd::signed_mode::BUILTIN, Hs...> make_keyring(flist11_t<Hs...>) {
  return {};
}

} // namespace k2o
