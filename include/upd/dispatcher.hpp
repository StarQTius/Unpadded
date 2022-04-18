//! \file

#pragma once

#include <type_traits>

#include "action.hpp"
#include "detail/io/immediate_process.hpp"
#include "detail/static_error.hpp"
#include "detail/type_traits/is_keyring.hpp"
#include "detail/type_traits/require.hpp"
#include "detail/type_traits/signature.hpp"
#include "detail/type_traits/typelist.hpp"
#include "flist.hpp"
#include "format.hpp"
#include "policy.hpp"
#include "tuple.hpp"
#include "unevaluated.hpp" // IWYU pragma: keep

#include "detail/def.hpp"

// IWYU pragma: no_forward_declare unevaluated

namespace upd {
namespace detail {

//! \name
//! \brief Make an action, with or without storage depending on the value of `Action_Features`
//! @{

template<action_features Action_Features,
         typename F,
         F Ftor,
         endianess Endianess,
         signed_mode Signed_Mode,
         REQUIRE(Action_Features == action_features::STATIC_STORAGE_DURATION_ONLY)>
no_storage_action make_action() {
  return no_storage_action{unevaluated<F, Ftor>{}, endianess_h<Endianess>{}, signed_mode_h<Signed_Mode>{}};
}

template<action_features Action_Features,
         typename F,
         F Ftor,
         endianess Endianess,
         signed_mode Signed_Mode,
         REQUIRE(Action_Features == action_features::ANY)>
action make_action() {
  return action{Ftor, endianess_h<Endianess>{}, signed_mode_h<Signed_Mode>{}};
}

//! @}

//! \brief Alias for `action` if `Action_Features` is `action_features::ANY`, `no_storage_action` otherwise
template<action_features Action_Features>
using action_t = decltype(make_action<Action_Features, int, 0, endianess::BUILTIN, signed_mode::BUILTIN>());

//! \brief Stores actions
//!
//! This class template helps with initializing the storage with a typelist
template<typename Index_T, Index_T Size, action_features Action_Features>
struct actions {
  template<typename... Fs, Fs... Ftors, endianess Endianess, signed_mode Signed_Mode>
  actions(flist_t<unevaluated<Fs, Ftors>...>, endianess_h<Endianess>, signed_mode_h<Signed_Mode>)
      : content{make_action<Action_Features, Fs, Ftors, Endianess, Signed_Mode>()...} {}

  action_t<Action_Features> content[Size];
};

} // namespace detail

//! \brief Action containers able to unserialize byte sequence serialized by an key
//!
//! A dispatcher is constructed from a keyring and is able to unserialize a payload serialized by
//! an key created from the same keyring. When it happens, the dispatcher calls the
//! function associated with the key, forwarding the arguments from the payload to the function. The
//! functions are internally held as `action` objects.
//!
//! \tparam Keyring Keyring describing the actions to manage
//! \tparam Action_Features Restriction on stored actions
template<typename Keyring, action_features Action_Features>
class dispatcher : public detail::immediate_process<dispatcher<Keyring, Action_Features>, typename Keyring::index_t> {
  static_assert(detail::is_keyring<Keyring>::value, K2O_ERROR_NOT_KEYRING(Keyring));

public:
  //! \copydoc keyring::signatures_t
  using signatures_t = typename Keyring::signatures_t;

  //! \copydoc keyring::index_t
  using index_t = typename Keyring::index_t;

  //! \copydoc detail::action_t
  using action_t = detail::action_t<Action_Features>;

  //! \copydoc keyring::size
  constexpr static auto size = Keyring::size;

  //! \copydoc keyring::endianess
  constexpr static auto endianess = Keyring::endianess;

  //!  \copydoc keyring::signed_mode
  constexpr static auto signed_mode = Keyring::signed_mode;

  //! \brief Construct the object from the provided keyring
  explicit dispatcher(Keyring, action_features_h<Action_Features>)
      : m_actions{typename Keyring::flist_t{}, endianess_h<endianess>{}, signed_mode_h<signed_mode>{}} {}

  using detail::immediate_process<dispatcher<Keyring, Action_Features>, index_t>::operator();

  //! \brief Extract an index from a byte sequence then invoke the action with that index
  //!
  //! The parameters for the action call are extracted from `src` and the return value is inserted into `dest`.
  //! \copydoc ImmediateProcess_CRTP
  //!
  //! \param src Input invocable
  //! \param dest Output invocable
  //! \return the index of the called action
  template<typename Src, typename Dest, REQUIREMENT(input_invocable, Src), REQUIREMENT(output_invocable, Dest)>
  index_t operator()(Src &&src, Dest &&dest) {
    auto index = get_index(src);

    if (index < size)
      m_actions.content[index](src, dest);

    return index;
  }

  //! \brief Extract an index from a byte sequence and get the action with that index
  //! \param src Input invocable
  //! \return Either a reference to the action if it exists or `nullptr`
  template<typename Src, REQUIREMENT(input_invocable, Src)>
  action_t *get_action(Src &&src) {
    auto index = get_index(FWD(src));
    return index < size ? m_actions.content + index : nullptr;
  }

  //! \brief Extract an index from a byte sequence
  //! \param src Input invocable
  //! \return The extracted index
  template<typename Src, REQUIREMENT(input_invocable, Src)>
  index_t get_index(Src &&src) const {
    tuple<endianess, signed_mode, index_t> index_tuple;

    for (auto &byte : index_tuple)
      byte = src();

    return get<0>(index_tuple);
  }

  //! \brief Replace with an invocable with static storage duration
  //! \tparam Index Index of the action to replace
  //! \tparam Ftor Invocable with static storage duration
  template<index_t Index, typename F, F Ftor>
  void replace(unevaluated<F, Ftor>) {
    static_assert(Index < size, K2O_ERROR_OUT_OF_BOUND(Index));
    static_assert(std::is_same<detail::at<signatures_t, Index>, detail::signature_t<F>>::value,
                  K2O_ERROR_SIGNATURE_MISMATCH(Ftor));

    m_actions.content[Index] = detail::make_action<Action_Features, F, Ftor, endianess, signed_mode>();
  }

#if __cplusplus >= 201703L
  //! \brief (C++17) Replace with an invocable with static storage duration
  //! \tparam Index Index of the action to replace
  //! \tparam Ftor Invocable with static storage duration
  template<index_t Index, auto &Ftor>
  void replace() {
    static_assert(Index < size, K2O_ERROR_OUT_OF_BOUND(Index));
    static_assert(std::is_same<detail::at<signatures_t, Index>, detail::signature_t<decltype(Ftor)>>::value,
                  K2O_ERROR_SIGNATURE_MISMATCH(Ftor));

    replace<Index>(unevaluated<decltype(Ftor) &, Ftor>{});
  }
#endif // __cplusplus >= 201703L

  //! \brief Replace with an invocable of any kind
  //! \tparam Index Index of the action to replace
  //! \param ftor Invocable of any kind
  template<index_t Index, typename F, REQUIRE_CLASS(Action_Features == action_features::ANY)>
  void replace(F &&ftor) {
    static_assert(Index < size, K2O_ERROR_OUT_OF_BOUND(Index));
    static_assert(std::is_same<detail::at<signatures_t, Index>, detail::signature_t<F>>::value,
                  K2O_ERROR_SIGNATURE_MISMATCH(ftor));

    m_actions.content[Index] = action{FWD(ftor), endianess_h<endianess>{}, signed_mode_h<signed_mode>{}};
  }

  //! \brief Get one of the stored actions
  //! \param index Index of an action
  //! \return the action associated with that index
  //! \warning No bound check is performed.
  action_t &operator[](index_t index) { return m_actions.content[index]; }

  //! \copydoc operator[]
  const action_t &operator[](index_t index) const { return m_actions.content[index]; }

private:
  detail::actions<index_t, size, Action_Features> m_actions;
};

//! \brief Make a dispatcher
//! \related dispatcher
template<typename Keyring, action_features Action_Features>
dispatcher<Keyring, Action_Features> make_dispatcher(Keyring, action_features_h<Action_Features>) {
  return dispatcher<Keyring, Action_Features>{Keyring{}, {}};
}

} // namespace upd

#include "detail/undef.hpp" // IWYU pragma: keep
