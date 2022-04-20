//! \file

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/detail/mp_fold.hpp>
#include <boost/mp11/detail/mp_list.hpp>
#include <boost/mp11/detail/mp_plus.hpp>
#include <boost/mp11/integer_sequence.hpp>
#include <boost/mp11/integral.hpp>
#include <boost/type_traits/conjunction.hpp>
#include <boost/type_traits/declval.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_volatile.hpp>

#include "detail/type_traits/require.hpp"
#include "detail/type_traits/signature.hpp"
#include "format.hpp"
#include "serialization.hpp"
#include "type.hpp"
#include "unaligned_data.hpp"

#include "detail/def.hpp"

namespace upd {

template<typename, endianess, signed_mode, typename...>
class tuple_view;

namespace detail {

template<typename, endianess, signed_mode, typename...>
class tuple_base; // IWYU pragma: keep

//! \name
//! \brief Normalize some type before being passed to a function
//!
//! For now, the only types to normalize are `std::array` instances (to their corresponding plain array-type)
// \warning Be careful when using these functions as they do not extent the lifetime of their parameter.
//!
//! @{

template<typename T, std::size_t N>
auto normalize(std::array<T, N> &&array) -> T(&&)[N] {
  return reinterpret_cast<T(&&)[N]>(*array.data());
}
template<typename T,
         require<std::is_same<decltype(read_as<T, endianess::BUILTIN, signed_mode::BUILTIN>(nullptr)), T>::value> = 0>
T &&normalize(T &&x) {
  return FWD(x);
}

//! @}

} // namespace detail

template<std::size_t I, typename D, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
decltype(boost::declval<detail::tuple_base<D, Endianess, Signed_Mode, Ts...>>().template get<I>())
get(const detail::tuple_base<D, Endianess, Signed_Mode, Ts...> &);

namespace detail {

//! \brief Return the size in bytes occupied by the serialization of instances of the provided type (if serializable)
template<typename T, detail::require_is_serializable<T> = 0>
constexpr std::size_t serialization_size_impl(...) {
  return sizeof(T);
}
template<typename T, detail::require_is_user_serializable<T> = 0>
constexpr std::size_t serialization_size_impl(int) {
  return decltype(make_view_for<endianess::BUILTIN, signed_mode::BUILTIN>(
      (byte_t *)nullptr, examine_invocable<decltype(upd_extension((T *)nullptr).unserialize)>{}))::size;
}

//! \brief Return the size in bytes occupied by the serialization of instances of the provided type (if serializable)
template<typename T>
struct serialization_size {
  constexpr static auto value = serialization_size_impl<T>(0);
};

//! \brief Make a tuple view according to a typelist
template<endianess Endianess, signed_mode Signed_Mode, typename It, typename... Ts>
auto make_view_from_typelist(const It &it, boost::mp11::mp_list<Ts...>)
    -> decltype(tuple_view<It, Endianess, Signed_Mode, Ts...>{it}) {
  return tuple_view<It, Endianess, Signed_Mode, Ts...>{it};
}

//! \brief Provides tuple features to the derived class throught CRTP
//! \tparam D Type of the derived class
//! \tparam Endianess endianess of the stored data
//! \tparam Signed_Mode signed mode of the stored data
//! \tparam Ts... Types of the serialized values
template<typename D, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
class tuple_base {
  D &derived() { return static_cast<D &>(*this); }
  const D &derived() const { return static_cast<const D &>(*this); }

  using typelist = boost::mp11::mp_list<Ts...>;
  using type_sizes = boost::mp11::mp_list<boost::mp11::mp_size_t<serialization_size<Ts>::value>...>;

  static_assert(
      boost::conjunction<std::integral_constant<bool,
                                                (!boost::is_const<Ts>::value && !boost::is_volatile<Ts>::value &&
                                                 !boost::is_reference<Ts>::value)>...>::value,
      "Type parameters cannot be cv-qualified or ref-qualified.");

  static_assert(boost::conjunction<detail::is_serializable<Ts>...>::value,
                "Some of the provided types are not serializable (serializable types are integer types, types with "
                "user-defined extension and array types of any of these)");

public:
  //! \brief Type of one of the serialized values
  //! \tparam I Index of the requested value's type
  template<std::size_t I>
  using arg_t = boost::mp11::mp_at_c<typelist, I>;

  //! \brief Storage size in byte
  constexpr static auto size = boost::mp11::mp_fold<type_sizes, boost::mp11::mp_size_t<0>, boost::mp11::mp_plus>::value;

  //! \brief Equals the endianess given as template parameter
  constexpr static auto storage_endianess = Endianess;

  //! \brief Equals the signed mode given as template parameter
  constexpr static auto storage_signed_mode = Signed_Mode;

  //! \brief Copy each element of a tuple-like object into the content
  //! \param t Tuple-like object to copy from
  template<typename T>
  D &operator=(T &&t) {
    using boost::mp11::index_sequence_for;

    lay_tuple(index_sequence_for<Ts...>{}, FWD(t));
    return derived();
  }

  //! \brief Unserialize one of the value held by the object
  //! \tparam I Index of the requested value
  //! \return A copy of the serialized value or an array if I designate an array type
#ifdef DOXYGEN
  template<std::size_t I>
  auto get() const;
#else
  template<std::size_t I>
  decltype(read_as<arg_t<I>, Endianess, Signed_Mode>(nullptr)) get() const {
    using namespace boost::mp11;
    constexpr auto offset = mp_fold<mp_take_c<type_sizes, I>, mp_size_t<0>, mp_plus>::value;

    return read_as<arg_t<I>, Endianess, Signed_Mode>(derived().src(), offset);
  }
#endif

  //! \brief Set one of the value held by the object
  //! \tparam I Index of the value which will be set
  //! \param value Value to be copied from
  template<std::size_t I>
  void set(const arg_t<I> &value) {
    using namespace boost::mp11;
    constexpr auto offset = mp_fold<mp_take_c<type_sizes, I>, mp_size_t<0>, mp_plus>::value;
    write_as<Endianess, Signed_Mode>(value, derived().src(), offset);
  }

  //! \brief Invoke a functor with the stored values
  //! \param ftor Functor to be invoked
  //! \return ftor(this->get<Is>()...) with Is = 0, 1, ..., sizeof...(Ts)
#ifdef DOXYGEN
  template<typename F>
  auto invoke(F &&ftor) const;
#else
  template<typename F>
  detail::return_t<F> invoke(F &&ftor) const {
    return invoke_impl(FWD(ftor), boost::mp11::index_sequence_for<Ts...>{});
  }
#endif

protected:
  //! \brief Serialize values into the object content
  template<std::size_t... Is, typename... Args>
  void lay(boost::mp11::index_sequence<Is...>, const Args &...args) {
    using discard = int[];
    discard{0, (set<Is>(args), 0)...};
  }

  //! \brief Lay the element of a tuple-like object into the content
  template<std::size_t... Is, typename T>
  void lay_tuple(boost::mp11::index_sequence<Is...> is, T &&t) {
    using upd::get;

    lay(is, get<Is>(FWD(t))...);
  }

  //! \brief Unserialize the tuple content and forward it as parameters to the provided functor
private:
  template<typename F, std::size_t... Is>
  detail::return_t<F> invoke_impl(F &&ftor, boost::mp11::index_sequence<Is...>) const {
    return FWD(ftor)(detail::normalize(get<Is>())...);
  }
};

} // namespace detail

//! \brief Call the member function 'tuple_base::get'
//! \details This function create a coherent interface 'std::tuple' for the sake of genericity
//! \param t Tuple to get a value from
template<std::size_t I, typename D, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
decltype(boost::declval<detail::tuple_base<D, Endianess, Signed_Mode, Ts...>>().template get<I>())
get(const detail::tuple_base<D, Endianess, Signed_Mode, Ts...> &t) {
  return t.template get<I>();
}

//! \brief Call the member function 'tuple_base::set'
//! \details This function create a coherent interface 'std::tuple' for the sake of genericity
//! \param t Tuple to set a value in
//! \param value Value to set
template<std::size_t I, typename D, endianess Endianess, signed_mode Signed_Mode, typename... Ts, typename U>
void set(detail::tuple_base<D, Endianess, Signed_Mode, Ts...> &t, U &&value) {
  return t.template set<I>(FWD(value));
}

//! \brief Binds a byte sequence to a tuple view
//! \details
//!   Once bound, the byte sequence content can be read and modified as it were the content of a 'tuple' object.
//!   The byte sequence and the tuple view are bound throught an iterator. It must at least be a forward iterator, but
//!   tuple views are faster with random access iterators. Best case would be a non-volatile plain pointer, as it can be
//!   called with memcpy.
//! \tparam It type of the iterators used for binding with the byte sequence
//! \tparam Endianess endianess of the stored data
//! \tparam Signed_Mode signed mode of the stored data
//! \tparam Ts... types of the serialized values
template<typename It, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
class tuple_view
    : public detail::tuple_base<tuple_view<It, Endianess, Signed_Mode, Ts...>, Endianess, Signed_Mode, Ts...> {
  using base_t = detail::tuple_base<tuple_view<It, Endianess, Signed_Mode, Ts...>, Endianess, Signed_Mode, Ts...>;

public:
  using base_t::operator=;

  //! \brief Bind the view to a byte sequence throught an iterator
  //! \param src Iterator to the start of the byte sequence
  explicit tuple_view(const It &src) : m_begin{src}, m_end{src} { std::advance(m_end, base_t::size); }

  //! \name Iterability
  //! @{
  const It &begin() const { return m_begin; }
  const It &end() const { return m_end; }
  //! @}

  //! \brief Get the iterator associated to the byte sequence
  const It &src() const { return m_begin; }

private:
  It m_begin, m_end;
};

//! \brief Bind a byte sequence to a tuple view
//! \tparam Ts... types held by the tuple
//! \tparam Endianess target endianess for serialization
//! \tparam Signed_Mode target signed representation for serialization
//! \param src start of the byte sequence
//! \return a 'tuple_view' object bound to the byte sequence
template<typename... Ts, typename It, endianess Endianess, signed_mode Signed_Mode>
tuple_view<It, Endianess, Signed_Mode, Ts...>
make_view(endianess_h<Endianess>, signed_mode_h<Signed_Mode>, const It &src) {
  return tuple_view<It, Endianess, Signed_Mode, Ts...>{src};
}

//! \brief Bind a byte sequence to a tuple view with native serialization parameters
//! \tparam Ts... types held by the tuple
//! \param src start of the byte sequence
//! \return a 'tuple_view' object bound to the byte sequence
template<typename... Ts, typename It>
tuple_view<It, endianess::BUILTIN, signed_mode::BUILTIN, Ts...> make_view(const It &src) {
  return tuple_view<It, endianess::BUILTIN, signed_mode::BUILTIN, Ts...>{src};
}

//! \upd_doc{MakeView_Tuple}
//! \brief Bind a view to a tuple
//! \tparam I First element in the view
//! \tparam L Number of elements in the view
//! \param tuple tuple to bind the view to
//! \return a tuple view including every elements in the given range

//! \copydoc MakeView_Tuple
template<std::size_t I, std::size_t L, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
auto make_view(tuple<Endianess, Signed_Mode, Ts...> &tuple) -> decltype(tuple.template view<I, L>()) {
  return tuple.template view<I, L>();
}
//! \copydoc MakeView_Tuple
template<std::size_t I, std::size_t L, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
auto make_view(const tuple<Endianess, Signed_Mode, Ts...> &tuple) -> decltype(tuple.template view<I, L>()) {
  return tuple.template view<I, L>();
}

//! \brief Unaligned storage with fixed target types
//! \details
//!   The object holds values of provided type in an unaligned maners (ie, there is no padding between two consecutive
//!   values).
//! \tparam Endianess endianess of the stored data
//! \tparam Signed_Mode signed mode of the stored data
//! \tparam Ts... Types of the serialized values
template<endianess Endianess = endianess::BUILTIN, signed_mode Signed_Mode = signed_mode::BUILTIN, typename... Ts>
class tuple : public detail::tuple_base<tuple<Endianess, Signed_Mode, Ts...>, Endianess, Signed_Mode, Ts...> {
  using base_t = detail::tuple_base<tuple<Endianess, Signed_Mode, Ts...>, Endianess, Signed_Mode, Ts...>;
  using typelist = boost::mp11::mp_list<Ts...>;
  using type_sizes = boost::mp11::mp_list<boost::mp11::mp_size_t<detail::serialization_size<Ts>::value>...>;

public:
  using base_t::operator=;

  //! \brief Initialize the content with default constructed value
  tuple() : tuple(Ts{}...) {}

  //! \brief Serialize the provided values
  //! \tparam Args... Serialized values types
  //! \param args... Values to be serialized
  explicit tuple(const Ts &...args) {
    using boost::mp11::index_sequence_for;
    base_t::lay(index_sequence_for<Ts...>{}, args...);
  }

  //! \name Iterability
  //! @{
  byte_t *begin() { return m_storage.begin(); }
  byte_t *end() { return m_storage.end(); }
  const byte_t *begin() const { return m_storage.begin(); }
  const byte_t *end() const { return m_storage.end(); }
  //! @}

  //! \brief Access the object content
  //! \details There is no bound check performed.
  //! \param i Index of the accessed byte
  byte_t &operator[](std::size_t i) { return m_storage[i]; }

  //! \brief Access the object content
  //! \details There is no bound check performed.
  //! \param i Index of the accessed byte
  const byte_t &operator[](std::size_t i) const { return m_storage[i]; }

#if __cplusplus >= 201703L
  //! \brief (C++17) Serialize the provided values
  //! \detail
  //!   Endianess and signed integer representation is provided throught the two first parameters.
  //! \param values... Values to be serialized
  //! \see format.hpp
  explicit tuple(endianess_h<Endianess>, signed_mode_h<Signed_Mode>, const Ts &...values) : tuple(values...) {}
#endif // __cplusplus >= 201703L

  //! \brief Get the 'begin' iterator
  byte_t *src() { return begin(); }
  const byte_t *src() const { return begin(); }

  //! \brief Make a view out of a subset of the tuple
  //! \tparam I First element in the view
  //! \tparam L Number of elements in the view
  //! \return a tuple view including every elements in the given range
  template<std::size_t I, std::size_t L>
  decltype(detail::make_view_from_typelist<Endianess, Signed_Mode>(
      (byte_t *)nullptr, boost::mp11::mp_take_c<boost::mp11::mp_drop_c<typelist, I>, L>{}))
  view() {
    using namespace boost::mp11;
    constexpr auto offset = mp_fold<mp_take_c<type_sizes, I>, mp_size_t<0>, mp_plus>::value;
    return detail::make_view_from_typelist<Endianess, Signed_Mode>(begin() + offset,
                                                                   mp_take_c<mp_drop_c<typelist, I>, L>{});
  }
  template<std::size_t I, std::size_t L>
  decltype(detail::make_view_from_typelist<Endianess, Signed_Mode>(
      (const byte_t *)nullptr, boost::mp11::mp_take_c<boost::mp11::mp_drop_c<typelist, I>, L>{}))
  view() const {
    using namespace boost::mp11;
    constexpr auto offset = mp_fold<mp_take_c<type_sizes, I>, mp_size_t<0>, mp_plus>::value;
    return detail::make_view_from_typelist<Endianess, Signed_Mode>(begin() + offset,
                                                                   mp_take_c<mp_drop_c<typelist, I>, L>{});
  }

private:
  unaligned_data<base_t::size, Endianess, Signed_Mode> m_storage;
};
template<endianess Endianess, signed_mode Signed_Mode>
class tuple<Endianess, Signed_Mode> : public detail::tuple_base<tuple<Endianess, Signed_Mode>, Endianess, Signed_Mode> {
public:
  //! \name Iterability
  //! @{
  constexpr byte_t *begin() const { return nullptr; }
  constexpr byte_t *end() const { return nullptr; }
  //! @}

  //! \brief Get the 'begin' iterator
  constexpr byte_t *src() const { return begin(); }
};

//! \brief Construct a tuple object provided constant lvalue to values
//! \tparam Endianess Target endianess for serialization
//! \tparam Signed_Mode Target signed representation for serialization
//! \tparam Args... Deduced types of the provided values.
//! \param args... Values to be serialized into the return value
//! \return tuple object holding a serialized copy of the provided values.
template<endianess Endianess, signed_mode Signed_Mode, typename... Args>
tuple<Endianess, Signed_Mode, Args...>
make_tuple(endianess_h<Endianess>, signed_mode_h<Signed_Mode>, const Args &...args) {
  return tuple<Endianess, Signed_Mode, Args...>{args...};
}

//! \brief Construct a tuple object provided constant lvalue to values using native data representation
//! \param args... Values to be serialized into the return value
//! \return a tuple object holding a serialized copy of the provided values.
template<typename... Args>
tuple<endianess::BUILTIN, signed_mode::BUILTIN, Args...> make_tuple(const Args &...args) {
  return tuple<endianess::BUILTIN, signed_mode::BUILTIN, Args...>{args...};
}

//! \brief Default construct a tuple object
//! \tparam Args... deduced types of the provided values
//! \tparam Endianess target endianess for serialization
//! \tparam Signed_Mode target signed representation for serialization
template<typename... Args, endianess Endianess, signed_mode Signed_Mode>
tuple<Endianess, Signed_Mode, Args...> make_tuple(endianess_h<Endianess>, signed_mode_h<Signed_Mode>) {
  return tuple<Endianess, Signed_Mode, Args...>{};
}

//! \brief Default construct a tuple object with native serialization parameters
//! \tparam Args... deduced types of the provided values
template<typename... Args>
tuple<endianess::BUILTIN, signed_mode::BUILTIN, Args...> make_tuple() {
  return tuple<endianess::BUILTIN, signed_mode::BUILTIN, Args...>{};
}

} // namespace upd

//! \brief Partial specialization of 'std::tuple_size' for 'tuple'
//! \detail
//!   This specialization enables to use structured binding with 'tuple'
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct std::tuple_size<upd::tuple<Endianess, Signed_Mode, Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

//! \brief Partial specialization of 'std::tuple_element' for 'tuple'
//! \detail
//!   This specialization enables to use structured binding with 'tuple'
template<std::size_t I, upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct std::tuple_element<I, upd::tuple<Endianess, Signed_Mode, Ts...>> {
  using type = decltype(boost::declval<upd::tuple<Endianess, Signed_Mode, Ts...>>().template get<I>());
};

#include "detail/undef.hpp" // IWYU pragma: keep
