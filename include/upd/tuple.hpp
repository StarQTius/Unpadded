//! \file

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

#include "detail/serialization.hpp"
#include "detail/type_traits/conjunction.hpp"
#include "detail/type_traits/index_sequence.hpp"
#include "detail/type_traits/require.hpp"
#include "detail/type_traits/signature.hpp"
#include "detail/type_traits/typelist.hpp"
#include "format.hpp"
#include "type.hpp"
#include "typelist.hpp"
#include "upd.hpp"

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

template<typename Target_T, typename T, std::size_t N, UPD_REQUIRE(std::is_same<Target_T, T[N]>::value)>
auto normalize(std::array<T, N> &&array) -> T (&&)[N] {
  return reinterpret_cast<T(&&)[N]>(*array.data());
}
template<typename Target_T,
         typename T,
         require<std::is_same<decltype(read_as<T, endianess::LITTLE, signed_mode::TWOS_COMPLEMENT>(nullptr)),
                              Target_T>::value> = 0>
T &&normalize(T &&x) {
  return UPD_FWD(x);
}

//! @}

//! \brief Return the size in bytes occupied by the serialization of instances of the provided type (if serializable)
template<typename T, detail::require_is_serializable<T> = 0>
constexpr std::size_t serialization_size_impl(...) {
  return sizeof(T);
}
template<typename T, detail::require_is_user_serializable<T> = 0>
constexpr std::size_t serialization_size_impl(int) {
  return decltype(make_view_for<endianess::LITTLE, signed_mode::TWOS_COMPLEMENT>(
      (byte_t *)nullptr, examine_invocable<decltype(upd_extension<T>::unserialize)>{}))::size;
}

//! \brief Return the size in bytes occupied by the serialization of instances of the provided type (if serializable)
template<typename T>
struct serialization_size {
  constexpr static auto value = serialization_size_impl<T>(0);
};

//! \brief Make a tuple view according to a typelist
template<endianess Endianess, signed_mode Signed_Mode, typename It, typename... Ts>
auto make_view_from_typelist(const It &it, detail::tlist_t<Ts...>)
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

  static_assert(detail::conjunction<std::integral_constant<bool,
                                                           (!std::is_const<Ts>::value && !std::is_volatile<Ts>::value &&
                                                            !std::is_reference<Ts>::value)>...>::value,
                "Type parameters cannot be cv-qualified or ref-qualified.");

  static_assert(detail::conjunction<detail::is_serializable<Ts>...>::value,
                "Some of the provided types are not serializable (serializable types are integer types, types with "
                "user-defined extension and array types of any of these)");

public:
  //! \brief Typelist holding `Ts...`
  using types_t = upd::typelist_t<Ts...>;

  //! \brief Typelist holding the sizes in byte of each type of `Ts...` when serialized
  using sizes_t = upd::typelist_t<serialization_size<Ts>...>;

  //! \brief Type of one of the serialized values
  //! \tparam I Index of the requested type in `Ts...`
  template<std::size_t I>
  using arg_t = detail::at<types_t, I>;

  //! \brief Storage size in byte
  constexpr static auto size = detail::sum<sizes_t>::value;

  //! \brief Equals the endianess given as template parameter
  constexpr static auto storage_endianess = Endianess;

  //! \brief Equals the signed mode given as template parameter
  constexpr static auto storage_signed_mode = Signed_Mode;

  //! \brief Copy each element of a tuple-like object into the content
  //! \param t Tuple-like object to copy from
  template<typename Tuple, UPD_REQUIREMENT(is_tuple, Tuple)>
  D &operator=(Tuple &&t) {
    lay_tuple(make_index_sequence<sizeof...(Ts)>{}, UPD_FWD(t));
    return derived();
  }

  //! \brief Unserialize one of the value held by the object
  //! \tparam I Index of the requested value
  //! \return A copy of the serialized value or a `std::array` instance if `I` designates an array type
#ifdef DOXYGEN
  template<std::size_t I>
  auto get() const;
#else
  template<std::size_t I>
  decltype(read_as<arg_t<I>, Endianess, Signed_Mode>(nullptr)) get() const {
    using detail::clip;
    using detail::sum;

    constexpr auto offset = sum<clip<sizes_t, 0, I>>::value;
    return read_as<arg_t<I>, Endianess, Signed_Mode>(derived().src(), offset);
  }
#endif

  //! \brief Set one of the value held by the object
  //! \tparam I Index of the value which will be set
  //! \param value Value to be copied from
  template<std::size_t I>
  void set(const arg_t<I> &value) {
    using detail::clip;
    using detail::sum;

    constexpr auto offset = sum<clip<sizes_t, 0, I>>::value;
    write_as<Endianess, Signed_Mode>(value, derived().src(), offset);
  }

  //! \brief Invoke a functor with the stored values
  //! \param ftor Callback to be invoked
  //! \return `ftor(upd::get<Is>()...)` with `Is` = `0`, `1`, ..., `sizeof...(Ts)`
#ifdef DOXYGEN
  template<typename F>
  auto invoke(F &&ftor) const;
#else
  template<typename F>
  detail::return_t<F> invoke(F &&ftor) const {
    return invoke_impl(UPD_FWD(ftor), make_index_sequence<sizeof...(Ts)>{});
  }
#endif

protected:
  //! \brief Serialize values into the object content
  template<std::size_t... Is, typename... Args>
  void lay(detail::index_sequence<Is...>, const Args &...args) {
    using discard = int[];
    (void)discard{0, (set<Is>(args), 0)...};
  }

  //! \brief Lay the element of a tuple-like object into the content
  template<std::size_t... Is, typename T>
  void lay_tuple(detail::index_sequence<Is...> is, T &&t) {
    lay(is, t.template get<Is>()...);
  }

private:
  //! \brief Unserialize the tuple content and forward it as parameters to the provided functor
  template<typename F, std::size_t... Is>
  detail::return_t<F> invoke_impl(F &&ftor, detail::index_sequence<Is...>) const {
    return UPD_FWD(ftor)(detail::normalize<Ts>(get<Is>())...);
  }
};

} // namespace detail

//! \brief Call the member function `get()` from an Unpadded tuple-like instance
//!
//! This function create a coherent interface with std::tuple for the sake of genericity.
//!
//! \param t Tuple to get a value from
#if defined(DOXYGEN)
template<std::size_t I, typename D, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
auto get(const detail::tuple_base<D, Endianess, Signed_Mode, Ts...> &t)
#else  // defined(DOXYGEN)
template<std::size_t I, typename D, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
decltype(std::declval<detail::tuple_base<D, Endianess, Signed_Mode, Ts...>>().template get<I>())
get(const detail::tuple_base<D, Endianess, Signed_Mode, Ts...> &t)
#endif // defined(DOXYGEN)
{
  return t.template get<I>();
}

//! \brief Call the member function `set()` from an Unpadded tuple-like instance
//!
//! This function create a coherent interface std::tuple for the sake of genericity
//!
//! \param t Tuple to set a value in
//! \param value Value to set
template<std::size_t I, typename D, endianess Endianess, signed_mode Signed_Mode, typename... Ts, typename U>
void set(detail::tuple_base<D, Endianess, Signed_Mode, Ts...> &t, U &&value) {
  return t.template set<I>(UPD_FWD(value));
}

//! \brief Binds a byte sequence to a tuple view
//!
//! Once bound, the byte sequence content can be read and modified as it were the content of a \ref<tuple> tuple
//! instance. The byte sequence and the tuple view are bound through an iterator. It must at least be a forward
//! iterator, but tuple views are faster with random access iterators. Best case would be a non-volatile plain pointer,
//! as it can be called with memcpy. \tparam It Type of the iterator used for binding with the byte sequence \tparam
//! Endianess, Signed_Mode Serialization parameters \tparam Ts... Types of the serialized values
template<typename It, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
class tuple_view
    : public detail::tuple_base<tuple_view<It, Endianess, Signed_Mode, Ts...>, Endianess, Signed_Mode, Ts...> {
  using base_t = detail::tuple_base<tuple_view<It, Endianess, Signed_Mode, Ts...>, Endianess, Signed_Mode, Ts...>;

public:
  using base_t::operator=;

  //! \brief Bind the view to a byte sequence through an iterator
  //! \param src Iterator to the start of the byte sequence
  explicit tuple_view(const It &src) : m_begin{src}, m_end{src} { std::advance(m_end, base_t::size); }

  //! \brief Beginning of the byte sequence
  const It &begin() const { return m_begin; }

  //! \brief End of the byte sequence
  const It &end() const { return m_end; }
  //! @}

  //! \copydoc begin()
  const It &src() const { return m_begin; }

private:
  It m_begin, m_end;
};

//! \brief Bind a byte sequence to a tuple view
//! \tparam Ts... Types held by the tuple
//! \tparam Endianess Target endianess for serialization
//! \tparam Signed_Mode Target signed representation for serialization
//! \param src Start of the byte sequence
//! \return a \ref<tuple_view> tuple_view instance bound to the byte sequence
//! \related tuple_view
template<typename... Ts, typename It, endianess Endianess, signed_mode Signed_Mode>
tuple_view<It, Endianess, Signed_Mode, Ts...>
make_view(endianess_h<Endianess>, signed_mode_h<Signed_Mode>, const It &src) {
  return tuple_view<It, Endianess, Signed_Mode, Ts...>{src};
}

//! \upd_doc{MakeView_Tuple}
//! \brief Bind a view to a slice of a \ref<tuple> tuple
//! \tparam I First element in the view
//! \tparam L Number of elements in the view
//! \param tuple Tuple to bind the view to
//! \return a \ref<tuple_view> tuple_view including every elements in the given range
//! \related tuple_view

//! \copydoc MakeView_Tuple
#if defined(DOXYGEN)
template<std::size_t I, std::size_t L, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
auto make_view(tuple<Endianess, Signed_Mode, Ts...> &tuple)
#else  // defined(DOXYGEN)
template<std::size_t I, std::size_t L, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
auto make_view(tuple<Endianess, Signed_Mode, Ts...> &tuple) -> decltype(tuple.template view<I, L>())
#endif // defined(DOXYGEN)
{
  return tuple.template view<I, L>();
}

//! \copydoc MakeView_Tuple
#if defined(DOXYGEN)
template<std::size_t I, std::size_t L, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
auto make_view(const tuple<Endianess, Signed_Mode, Ts...> &tuple)
#else  // defined(DOXYGEN)
template<std::size_t I, std::size_t L, endianess Endianess, signed_mode Signed_Mode, typename... Ts>
auto make_view(const tuple<Endianess, Signed_Mode, Ts...> &tuple) -> decltype(tuple.template view<I, L>())
#endif // defined(DOXYGEN)
{
  return tuple.template view<I, L>();
}

//! \brief Unaligned storage tuple
//!
//! \ref<tuple> tuple instances hold values like a tuple but in an unaligned manner (i.e., there is no padding between
//! two consecutive values).
//!
//! \tparam Endianess, Signed_Mode Serialization parameters
//! \tparam Ts... Types of the serialized values
template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
class tuple : public detail::tuple_base<tuple<Endianess, Signed_Mode, Ts...>, Endianess, Signed_Mode, Ts...> {
  using base_t = detail::tuple_base<tuple<Endianess, Signed_Mode, Ts...>, Endianess, Signed_Mode, Ts...>;
  using types_t = typename base_t::types_t;
  using sizes_t = typename base_t::sizes_t;

public:
  using base_t::operator=;

  //! \brief Initialize the internal storage with default constructed values
  tuple() : tuple(Ts{}...) {}

  //! \brief Serialize the provided values
  //! \param args... Values to be serialized
  explicit tuple(const Ts &...args) { base_t::lay(detail::make_index_sequence<sizeof...(Ts)>{}, args...); }

#if __cplusplus >= 201703L
  //! \brief (C++17) Serialize the provided values
  //!
  //! \tparam Endianess, Signed_Mode Serialization parameters
  //! \param values... Values to be serialized
  explicit tuple(endianess_h<Endianess>, signed_mode_h<Signed_Mode>, const Ts &...values) : tuple(values...) {}
#endif // __cplusplus >= 201703L

  //! \brief Beginning of the internal storage
  byte_t *begin() { return m_storage; }

  //! \copydoc begin()
  const byte_t *begin() const { return m_storage; }

  //! \brief End of the internal storage
  byte_t *end() { return m_storage + base_t::size; }

  //! \copydoc end()
  const byte_t *end() const { return m_storage + base_t::size; }

  //! \brief Access the object content
  //!
  //! \warning There is no bound check performed.
  //!
  //! \param i Index of the accessed byte
  byte_t &operator[](std::size_t i) { return m_storage[i]; }

  //! \brief Access the object content
  //!
  //! \warning There is no bound check performed.
  //!
  //! \param i Index of the accessed byte
  const byte_t &operator[](std::size_t i) const { return m_storage[i]; }

  //! \copydoc begin()
  byte_t *src() { return begin(); }

  //! \copydoc begin()
  const byte_t *src() const { return begin(); }

  //! \brief Make a view out of a subset of the tuple
  //! \tparam I First element in the view
  //! \tparam L Number of elements in the view
  //! \return a \ref<tuple_view> tuple_view instance including every elements in the given range
#if defined(DOXYGEN)
  template<std::size_t I, std::size_t L>
  auto view()
#else  // defined(DOXYGEN)
  template<std::size_t I, std::size_t L>
  decltype(detail::make_view_from_typelist<Endianess, Signed_Mode>((byte_t *)nullptr, detail::clip<types_t, I, L>{}))
  view()
#endif // defined(DOXYGEN)
  {
    using detail::clip;
    using detail::sum;

    constexpr auto offset = sum<clip<sizes_t, 0, I>>::value;

    return detail::make_view_from_typelist<Endianess, Signed_Mode>(begin() + offset, clip<types_t, I, L>{});
  }

  //! \copydoc view
#if defined(DOXYGEN)
  template<std::size_t I, std::size_t L>
  auto view() const
#else  // defined(DOXYGEN)
  template<std::size_t I, std::size_t L>
  decltype(detail::make_view_from_typelist<Endianess, Signed_Mode>((const byte_t *)nullptr,
                                                                   detail::clip<types_t, I, L>{}))
  view() const
#endif // defined(DOXYGEN)
  {
    using detail::clip;
    using detail::sum;
    constexpr auto offset = sum<clip<sizes_t, 0, I>>::value;
    return detail::make_view_from_typelist<Endianess, Signed_Mode>(begin() + offset, clip<types_t, I, L>{});
  }

private:
  upd::byte_t m_storage[base_t::size];
};

template<endianess Endianess, signed_mode Signed_Mode>
class tuple<Endianess, Signed_Mode> : public detail::tuple_base<tuple<Endianess, Signed_Mode>, Endianess, Signed_Mode> {
public:
  constexpr byte_t *begin() const { return nullptr; }
  constexpr byte_t *end() const { return nullptr; }
  constexpr byte_t *src() const { return begin(); }
};

//! \brief Construct a \ref<tuple> tuple instance from provided values
//! \tparam Endianess, Signed_Mode Serialization parameters
//! \param args... Values to be serialized into the return value
//! \return a \ref<tuple> tuple instance initialized from `args...`
//! \related tuple
template<endianess Endianess, signed_mode Signed_Mode, typename... Args>
tuple<Endianess, Signed_Mode, Args...>
make_tuple(endianess_h<Endianess>, signed_mode_h<Signed_Mode>, const Args &...args) {
  return tuple<Endianess, Signed_Mode, Args...>{args...};
}

//! \brief Construct a \ref<tuple> tuple instance holding default values
//! \tparam Args... Types of the values held by the tuple
//! \tparam Endianess, Signed_Mode Serialization parameters
//! \related tuple
template<typename... Args, endianess Endianess, signed_mode Signed_Mode>
tuple<Endianess, Signed_Mode, Args...> make_tuple(endianess_h<Endianess>, signed_mode_h<Signed_Mode>) {
  return tuple<Endianess, Signed_Mode, Args...>{};
}

} // namespace upd

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct std::tuple_size<upd::tuple<Endianess, Signed_Mode, Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

template<std::size_t I, upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct std::tuple_element<I, upd::tuple<Endianess, Signed_Mode, Ts...>> {
  using type = decltype(std::declval<upd::tuple<Endianess, Signed_Mode, Ts...>>().template get<I>());
};

template<typename It, upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct std::tuple_size<upd::tuple_view<It, Endianess, Signed_Mode, Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

template<std::size_t I, typename It, upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct std::tuple_element<I, upd::tuple_view<It, Endianess, Signed_Mode, Ts...>> {
  using type = decltype(std::declval<upd::tuple<Endianess, Signed_Mode, Ts...>>().template get<I>());
};
