//! \file

#pragma once

#include <type_traits>

#include <upd/format.hpp>
#include <upd/tuple.hpp>

#include "detail/io/immediate_process.hpp"
#include "detail/static_error.hpp"
#include "detail/type_traits/is_keyring.hpp"
#include "detail/type_traits/require.hpp"
#include "detail/type_traits/signature.hpp"
#include "detail/type_traits/typelist.hpp"
#include "flist.hpp"
#include "order.hpp"
#include "policy.hpp"
#include "unevaluated.hpp" // IWYU pragma: keep

#include "detail/def.hpp"

// IWYU pragma: no_forward_declare unevaluated

namespace k2o {
namespace detail {

//! \name
//! \brief Make an order, with or without storage depending on the value of `Order_Features`
//! @{

template<order_features Order_Features,
         typename F,
         F Ftor,
         upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         REQUIRE(Order_Features == order_features::STATIC_STORAGE_DURATION_ONLY)>
no_storage_order make_order() {
  return no_storage_order{unevaluated<F, Ftor>{}, upd::endianess_h<Endianess>{}, upd::signed_mode_h<Signed_Mode>{}};
}

template<order_features Order_Features,
         typename F,
         F Ftor,
         upd::endianess Endianess,
         upd::signed_mode Signed_Mode,
         REQUIRE(Order_Features == order_features::ANY)>
order make_order() {
  return order{Ftor, upd::endianess_h<Endianess>{}, upd::signed_mode_h<Signed_Mode>{}};
}

//! @}

//! \brief Alias for `order` if `Order_Features` is `order_features::ANY`, `no_storage_order` otherwise
template<order_features Order_Features>
using order_t = decltype(make_order<Order_Features, int, 0, upd::endianess::BUILTIN, upd::signed_mode::BUILTIN>());

//! \brief Stores orders
//!
//! This class template helps with initializing the storage with a typelist
template<typename Index_T, Index_T Size, order_features Order_Features>
struct orders {
  template<typename... Fs, Fs... Ftors, upd::endianess Endianess, upd::signed_mode Signed_Mode>
  orders(flist_t<unevaluated<Fs, Ftors>...>, upd::endianess_h<Endianess>, upd::signed_mode_h<Signed_Mode>)
      : content{make_order<Order_Features, Fs, Ftors, Endianess, Signed_Mode>()...} {}

  order_t<Order_Features> content[Size];
};

} // namespace detail

//! \brief Order containers able to unserialize byte sequence serialized by an key
//!
//! A dispatcher is constructed from a keyring and is able to unserialize a payload serialized by
//! an key created from the same keyring. When it happens, the dispatcher calls the
//! function associated with the key, forwarding the arguments from the payload to the function. The
//! functions are internally held as `order` objects.
//!
//! \tparam Keyring Keyring describing the orders to manage
//! \tparam Order_Features Restriction on stored orders
template<typename Keyring, order_features Order_Features>
class dispatcher : public detail::immediate_process<dispatcher<Keyring, Order_Features>, typename Keyring::index_t> {
  static_assert(detail::is_keyring<Keyring>::value, K2O_ERROR_NOT_KEYRING(Keyring));

public:
  //! \copydoc keyring::signatures_t
  using signatures_t = typename Keyring::signatures_t;

  //! \copydoc keyring::index_t
  using index_t = typename Keyring::index_t;

  //! \copydoc detail::order_t
  using order_t = detail::order_t<Order_Features>;

  //! \copydoc keyring::size
  constexpr static auto size = Keyring::size;

  //! \copydoc keyring::endianess
  constexpr static auto endianess = Keyring::endianess;

  //!  \copydoc keyring::signed_mode
  constexpr static auto signed_mode = Keyring::signed_mode;

  //! \brief Construct the object from the provided keyring
  explicit dispatcher(Keyring, order_features_h<Order_Features>)
      : m_orders{typename Keyring::flist_t{}, upd::endianess_h<endianess>{}, upd::signed_mode_h<signed_mode>{}} {}

  using detail::immediate_process<dispatcher<Keyring, Order_Features>, index_t>::operator();

  //! \brief Extract an index from a byte sequence then invoke the order with that index
  //!
  //! The parameters for the order call are extracted from `src` and the return value is inserted into `dest`.
  //! \copydoc ImmediateProcess_CRTP
  //!
  //! \param src Input invocable
  //! \param dest Output invocable
  //! \return the index of the called order
  template<typename Src, typename Dest, REQUIREMENT(input_invocable, Src), REQUIREMENT(output_invocable, Dest)>
  index_t operator()(Src &&src, Dest &&dest) {
    auto index = get_index(src);

    if (index < size)
      m_orders.content[index](src, dest);

    return index;
  }

  //! \brief Extract an index from a byte sequence and get the order with that index
  //! \param src Input invocable
  //! \return Either a reference to the order if it exists or `nullptr`
  template<typename Src, REQUIREMENT(input_invocable, Src)>
  order_t *get_order(Src &&src) {
    auto index = get_index(FWD(src));
    return index < size ? m_orders.content + index : nullptr;
  }

  //! \brief Extract an index from a byte sequence
  //! \param src Input invocable
  //! \return The extracted index
  template<typename Src, REQUIREMENT(input_invocable, Src)>
  index_t get_index(Src &&src) const {
    upd::tuple<endianess, signed_mode, index_t> index_tuple;

    for (auto &byte : index_tuple)
      byte = src();

    return get<0>(index_tuple);
  }

  //! \brief Replace with an invocable with static storage duration
  //! \tparam Index Index of the order to replace
  //! \tparam Ftor Invocable with static storage duration
  template<index_t Index, typename F, F Ftor>
  void replace(unevaluated<F, Ftor>) {
    static_assert(Index < size, K2O_ERROR_OUT_OF_BOUND(Index));
    static_assert(std::is_same<detail::at<signatures_t, Index>, detail::signature_t<F>>::value,
                  K2O_ERROR_SIGNATURE_MISMATCH(Ftor));

    m_orders.content[Index] = detail::make_order<Order_Features, F, Ftor, endianess, signed_mode>();
  }

#if __cplusplus >= 201703L
  //! \brief (C++17) Replace with an invocable with static storage duration
  //! \tparam Index Index of the order to replace
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
  //! \tparam Index Index of the order to replace
  //! \param ftor Invocable of any kind
  template<index_t Index, typename F, REQUIRE_CLASS(Order_Features == order_features::ANY)>
  void replace(F &&ftor) {
    static_assert(Index < size, K2O_ERROR_OUT_OF_BOUND(Index));
    static_assert(std::is_same<detail::at<signatures_t, Index>, detail::signature_t<F>>::value,
                  K2O_ERROR_SIGNATURE_MISMATCH(ftor));

    m_orders.content[Index] = order{FWD(ftor), upd::endianess_h<endianess>{}, upd::signed_mode_h<signed_mode>{}};
  }

  //! \brief Get one of the stored orders
  //! \param index Index of an order
  //! \return the order associated with that index
  //! \warning No bound check is performed.
  order_t &operator[](index_t index) { return m_orders.content[index]; }

  //! \copydoc operator[]
  const order_t &operator[](index_t index) const { return m_orders.content[index]; }

private:
  detail::orders<index_t, size, Order_Features> m_orders;
};

//! \brief Make a dispatcher
//! \related dispatcher
template<typename Keyring, order_features Order_Features>
dispatcher<Keyring, Order_Features> make_dispatcher(Keyring, order_features_h<Order_Features>) {
  return dispatcher<Keyring, Order_Features>{Keyring{}, {}};
}

} // namespace k2o

#include "detail/undef.hpp" // IWYU pragma: keep
