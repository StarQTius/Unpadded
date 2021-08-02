#pragma once

#include <tuple>

#include "boost/mp11.hpp"
#include "boost/type_traits.hpp"

#include "upd/format.hpp"
#include "upd/fwd.hpp"
#include "upd/type.hpp"

#include "upd/detail/signature.hpp"

#include "unaligned_data.hpp"

/*!
  \file
  \brief Definition of the tuple class
*/

namespace upd {

//! \brief Defines base members for 'tuple'
//! \tparam Endianess endianess of the stored data
//! \tparam Signed_Mode signed mode of the stored data
//! \tparam Ts... Types of the serialized values
template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
class tuple_base {
  using typelist = boost::mp11::mp_list<Ts...>;
  using type_sizes = boost::mp11::mp_list<boost::mp11::mp_size_t<sizeof(Ts)>...>;

  static_assert(boost::conjunction<
    boost::integral_constant<bool,(
      !boost::is_const<Ts>::value &&
      !boost::is_volatile<Ts>::value &&
      !boost::is_reference<Ts>::value)>...>::value,
    "Type parameters cannot be cv-qualified or ref-qualified.");

public:
  //! \brief Type of one of the serialized values
  //! \tparam I Index of the requested value's type
  template<size_t I>
  using arg_t = boost::mp11::mp_at_c<typelist, I>;

  //! \brief Storage size in byte
  constexpr static auto size = boost::mp11::mp_fold<type_sizes, boost::mp11::mp_size_t<0>, boost::mp11::mp_plus>::value;

  //! \brief Equals the endianess given as template parameter
  constexpr static auto storage_endianess = Endianess;

  //! \brief Equals the signed mode given as template parameter
  constexpr static auto storage_signed_mode = Signed_Mode;

  /*!
    \brief Access the object content
    \details There is no bound check performed.
    \param i Index of the accessed byte
  */
  byte_t& operator[](size_t i) { return m_storage[i]; }

  /*!
    \brief Access the object content
    \details There is no bound check performed.
    \param i Index of the accessed byte
  */
  const byte_t& operator[](size_t i) const { return m_storage[i]; }

  /*!
    \name Iterability
    @{
  */

  byte_t* begin() { return m_storage.begin(); }
  byte_t* end() { return m_storage.end(); }
  const byte_t* begin() const { return m_storage.begin(); }
  const byte_t* end() const { return m_storage.end(); }

  //! @}

  /*!
    \brief Unserialize one of the value held by the object
    \tparam I Index of the requested value
    \return A copy of the serialized value or an array_wrapper if I designate an array type
  */
#ifdef DOXYGEN
  template<size_t I> auto get() const;
#else
  template<size_t I>
  decltype(boost::declval<unaligned_data<size, Endianess, Signed_Mode>>().template interpret_as<arg_t<I>>(0))
  get() const {
    using namespace boost::mp11;
    constexpr auto offset = mp_fold<mp_take_c<type_sizes, I>, mp_size_t<0>, mp_plus>::value;

    return m_storage.template interpret_as<arg_t<I>>(offset);
  }
#endif

  /*!
    \brief Set one of the value held by the object
    \tparam I Index of the value which will be set
    \param value Value to be copied from
  */
  template<size_t I>
  void set(const arg_t<I>& value) {
    using namespace boost::mp11;
    constexpr auto offset = mp_fold<mp_take_c<type_sizes, I>, mp_size_t<0>, mp_plus>::value;
    m_storage.write(value, offset);
  }

  /*!
    \brief Invoke a functor with the stored values
    \param ftor Functor to be invoked
    \return ftor(this->get<Is>()...) with Is = 0, 1, ..., sizeof...(Ts)
  */
#ifdef DOXYGEN
  template<typename F> auto invoke(F&& ftor) const;
#else
  template<typename F>
  detail::return_t<F> invoke(F&& ftor) const {
    return invoke_impl(UPD_FWD(ftor), boost::mp11::index_sequence_for<Ts...>{});
  }
#endif

protected:
  template<size_t... Is, typename... Args>
  void lay(boost::mp11::index_sequence<Is...>, const Args&... args) {
     // TODO : à changer pour quelque chose de plus propre
     using discard = int[];
     discard {0, (set<Is>(args), 0)...};
  }

private:
  template<typename F, size_t... Is>
  detail::return_t<F> invoke_impl(F&& ftor, boost::mp11::index_sequence<Is...>) const {
    return UPD_FWD(ftor)(get<Is>()...);
  }

  unaligned_data<size, Endianess, Signed_Mode> m_storage;

};

/*!
  \brief Unaligned storage with fixed target types
  \details
    The object holds values of provided type in an unaligned maners (ie, there is no padding between two consecutive
    values).
  \tparam Endianess endianess of the stored data
  \tparam Signed_Mode signed mode of the stored data
  \tparam Ts... Types of the serialized values
*/
template<endianess Endianess = endianess::BUILTIN, signed_mode Signed_Mode = signed_mode::BUILTIN, typename... Ts>
class tuple : public tuple_base<Endianess, Signed_Mode, Ts...> {
public:
  /*!
    \brief Initialize the content with default constructed value
  */
  tuple() : tuple(Ts{}...) {}

  /*!
    \brief Serialize the provided values
    \tparam Args... Serialized values types
    \param args... Values to be serialized
  */
  explicit tuple(const Ts&... args) {
    using boost::mp11::index_sequence_for;
    tuple_base<Endianess, Signed_Mode, Ts...>::lay(index_sequence_for<Ts...>{}, args...);
  }

#if __cplusplus >= 201703L
  //! \brief (C++17) Serialize the provided values
  //! \detail
  //!   Endianess and signed integer representation is provided throught the two first parameters.
  //! \param values... Values to be serialized
  //! \see format.hpp
  explicit tuple(value_h<endianess, Endianess>, value_h<signed_mode, Signed_Mode>, const Ts&... values) :
    tuple(values...) {}
#endif // __cplusplus >= 201703L
};

//! \brief Enables to instantiate the class template with no type parameters
//! \tparam Endianess endianess of the stored data
//! \tparam Signed_Mode signed mode of the stored data
template<endianess Endianess, signed_mode Signed_Mode>
class tuple<Endianess, Signed_Mode> : public tuple_base<Endianess, Signed_Mode> {};

/*!
  \brief Construct a tuple object provided constant lvalue to values
  \tparam Endianess Target endianess for serialization
  \tparam Signed_Mode Target signed representation for serialization
  \tparam Args... Deduced types of the provided values.
  \param args... Values to be serialized into the return value
  \return tuple object holding a serialized copy of the provided values.
*/
template<endianess Endianess = endianess::BUILTIN, signed_mode Signed_Mode = signed_mode::BUILTIN, typename... Args>
tuple<Endianess, Signed_Mode, Args...> make_tuple(const Args&... args) {
  return tuple<Endianess, Signed_Mode, Args...>{args...};
}

//! \brief Default construct a tuple object
//! \tparam Args... deduced types of the provided values
//! \tparam Endianess target endianess for serialization
//! \tparam Signed_Mode target signed representation for serialization
template<typename... Args, endianess Endianess, signed_mode Signed_Mode>
tuple<Endianess, Signed_Mode, Args...> make_tuple(value_h<endianess, Endianess>, value_h<signed_mode, Signed_Mode>) {
  return tuple<Endianess, Signed_Mode, Args...>{};
}

//! \brief Default construct a tuple object with native serialization parameters
//! \tparam Args... deduced types of the provided values
template<typename... Args>
tuple<endianess::BUILTIN, signed_mode::BUILTIN, Args...> make_tuple() {
  return tuple<endianess::BUILTIN, signed_mode::BUILTIN, Args...>{};
}

} // namespace upd

//! \brief Partial specialization of 'std::tuple_size' for 'upd::tuple'
//! \detail
//!   This specialization enables to use structured binding with 'upd::tuple'
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct std::tuple_size<upd::tuple<Endianess, Signed_Mode, Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

//! \brief Partial specialization of 'std::tuple_element' for 'upd::tuple'
//! \detail
//!   This specialization enables to use structured binding with 'upd::tuple'
template<size_t I, upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct std::tuple_element<I, upd::tuple<Endianess, Signed_Mode, Ts...>> {
  using type = decltype(boost::declval<upd::tuple<Endianess, Signed_Mode, Ts...>>().template get<I>());
};
