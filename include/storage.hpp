#pragma once

#include <cstring>

#include "ct_magic.hpp"

#include "type.hpp"
#include "endianess.hpp"

/*!
  \file
  \brief Definitions of unaligned storage class for serializing value.
*/

namespace upd {
namespace concept {

/*!
  \brief Utility class for SFINAE to check if a type is an integer
*/
template<typename T, typename U = void>
using require_integer = ctm::enable_t<ctm::category::integer.has<T>(), U>;

/*!
  \brief Utility class for SFINAE to check if a type is an array type
*/
template<typename T, typename U = void>
using require_array = ctm::enable_t<ctm::is_array<T>::value, U>;

} // namespace concept

namespace detail {

/*!
  \brief Perform a reversed memcpy
  \param dest Destination of the copy
  \param src Source of the copy
  \param len Number of byte read from src
*/
inline void rmemcpy(byte_t* dest, const byte_t* src, size_t len) {
  for (size_t i = 0; i < len; i++)
    dest[i] = src[len - i - 1];
}

} // namespace detail

/*!
  \brief Utility class wrapping an C-style array, analogously to std::array
*/
template<typename T, size_t N>
struct array_wrapper {
  //! \brief Type of the contained elements
  using type = T;

  //! \brief Size of the container
  constexpr static auto size = N;

  /*!
    \brief Initialize the object's content of the object
  */
  array_wrapper() = default;

  /*!
    \brief Copy from a buffer to the object's content of the object
    \param ptr Pointer to the buffer
  */
  explicit array_wrapper(const T* ptr) { memcpy(content, ptr, N); }

  /*!
    \name Array-like operators
    @{
  */

  T& operator[](size_t i) { return content[i]; }
  const T& operator[](size_t i) const { return content[i]; }

  T& operator*() { return *content; }
  const T& operator*() const { return *content; }

  T* operator->() { return content; }
  const T* operator->() const { return content; }

  T* operator+(size_t n) { return content + n; }
  const T* operator+(size_t n) const { return content + n; }

  //! @}

  //! \brief Object's content
  //! \details Every operation on the object has the same result on the object's content.
  T content[N];
};

/*!
  \brief Equivalent array_wrapper of the given array type
  \tparam T Given array type
*/
template<typename T>
using wrap_array = array_wrapper<ctm::element_t<T>, ctm::introspect_array<T>::size>;

/*!
  \brief Unaligned storage enabling reading and writing at any offset
  \details
    The class holds an array of bytes used to store the serialized representation of values without padding due
    to memory alignment.
    The only reasonably serializable value are integer values; Thus the implementation of unaligned_data only support
    integer values. However, it doesn't handle signed representation yet.
    The user shall provide the target endianess so that the integer are serialized without depending on the platform
    endianess.
  \tparam N Size of the content in bytes
*/
template<size_t N>
class unaligned_data {
public:
  /*!
    \brief Storage size in byte
  */
  constexpr static auto size = N;

  /*!
    \name Iterators
    @{
  */

  /*!
    \brief Iterator for accessing unaligned_data internal storage
    \details It satisfies the forward iterator requirement.
  */
  class iterator {
  public:
    iterator() : ptr{nullptr} {}

    explicit iterator(byte_t* ptr) : ptr{ptr} {}

    iterator operator++() {
      iterator previous;
      previous.ptr = ptr;
      ptr++;
      return previous;
    }

    byte_t& operator*() const { return *ptr; }
    bool operator!=(const iterator& other) const { return ptr != other.ptr; }

  private:
    byte_t* ptr;

  };

  /*!
    \brief Construct the object without initializing the object content
    \details
      It satisfies the forward iterator requirement.
      It is a constant iterator, meaning it permits read-only access to the container.
  */
  class const_iterator {
  public:
    const_iterator() : ptr{nullptr} {}

    explicit const_iterator(const byte_t* ptr) : ptr{ptr} {}

    const_iterator operator++() {
      const_iterator previous;
      previous.ptr = ptr;
      ptr++;
      return previous;
    }

    const byte_t& operator*() const { return *ptr; }
    bool operator!=(const iterator& other) const { return ptr != other.ptr; }

  private:
    const byte_t* ptr;

  };
  //! @}

  /*!
    \brief Default initialize the object content
    \param endianess Target endianess for serialization
  */
  explicit unaligned_data(system::endianess endianess) : m_endianess{endianess} {};

  /*!
    \brief Initialize the object content of the object by copying from a buffer
    \details The Nth first bytes of the buffer are copied.
    \param raw_data Pointer to the buffer
    \param endianess Target endianess for serialization
  */
  explicit unaligned_data(const byte_t* raw_data, system::endianess endianess) : m_endianess{endianess} {
    memcpy(m_raw_data, raw_data, N);
  }

  /*!
    \name Iterability
    @{
  */
  const iterator begin() { return iterator{m_raw_data}; }
  const iterator end() { return iterator{m_raw_data + N}; }
  const const_iterator begin() const { return const_iterator{m_raw_data}; }
  const const_iterator end() const { return const_iterator{m_raw_data + N}; }
  //! @}

  /*!
    \brief Access the object content
    \details There is no bound check performed.
    \param i Index of the accessed byte
  */
  byte_t& operator[](size_t i) { return raw_data[i]; }

  /*!
    \brief Access the object content
    \details There is no bound check performed.
    \param i Index of the accessed byte
  */
  const byte_t& operator[](size_t i) const { return raw_data[i]; }

  /*!
    \brief Interpret a part of the object content as the given type
    \details
      There is no bound check performed.
      This overload kicks in when T is an integer type.
    \tparam T Requested type
    \param i Offset of the object content to be interpreted
    \return A copy of the value represented by the raw data at the given offset
  */
#ifdef DOXYGEN
  template<typename T> T interpret_as(size_t i) const;
#else
  template<typename T>
  concept::require_integer<T, T>
  interpret_as(size_t i) const {
    union { T base; byte_t raw[sizeof(T)]; } byte_rep;
    if (system::platform_endianess == m_endianess)
      memcpy(byte_rep.raw, m_raw_data + i, sizeof(T));
    else
      detail::rmemcpy(byte_rep.raw, m_raw_data + i, sizeof(T));
    return byte_rep.base;
  }
#endif

  /*!
    \brief Interpret a part of the object content as the given type
    \details
      There is no bound check performed.
      This overload kicks in when T is the type of an array of integer.
    \tparam T Requested type
    \param i Offset of the object content to be interpreted
    \return An array_wrapper object containing a copy of the requested array
  */
#ifdef DOXYGEN
  template<typename T> wrap_array<T> interpret_as(size_t i) const;
#else
  template<typename T>
  concept::require_array<T, wrap_array<T>>
  interpret_as(size_t i) const {
    wrap_array<T> array_wrapper;

    using element_t = typename decltype(array_wrapper)::type;
    constexpr auto size = decltype(array_wrapper)::size;

    for (size_t j = 0; j < size; j++)
      array_wrapper[j] = interpret_as<element_t>(i + j * sizeof(element_t));

    return array_wrapper;
  }
#endif

  /*!
    \brief Serialize a value into the object's content
    \details
      There is no bound check performed.
      This overload kicks in when T is an integer type.
    \tparam T Serilized value's type
    \param x Value to be serialized
    \param i Offset where the value will be serialized
  */
#ifdef DOXYGEN
  template<typename T> void write(const T& x, size_t i);
#else
  template<typename T>
  concept::require_integer<T>
  write(const T& x, size_t i) {
    if (system::platform_endianess == m_endianess)
      memcpy(m_raw_data + i, reinterpret_cast<const void*>(&x), sizeof(T));
    else
      detail::rmemcpy(m_raw_data + i, reinterpret_cast<const byte_t*>(&x), sizeof(T));
  }
#endif

  /*!
    \brief Serialize a value into the object's content
    \details
      There is no bound check performed.
      This overload kicks in when T is the type of an array of integer.
    \tparam T Serilized value's type
    \param x Value to be serialized
    \param i Offset where the value will be serialized
  */
#ifdef DOXYGEN
  template<typename T> void write(const T& array, size_t i)
#else
  template<typename T>
  concept::require_array<T>
  write(const T& array, size_t i) {
    using element_t = ctm::element_t<T>;
    constexpr auto array_size = ctm::introspect_array<T>::size;
    for (size_t j = 0; j < array_size; j++) write(array[j], i + j * sizeof(element_t));
  }
#endif

  /*!
    \brief Give a read-only view to the object's content
    \return A pointer to the beginning of the object's content
  */
  const byte_t* raw_data() const { return m_raw_data; }

private:
  byte_t m_raw_data[N];
  system::endianess m_endianess;

};

/*!
  \brief Construct an unaligned_data object provided a lvalue to a bounded array
  \tparam N Size of the bounded array
  \param raw_data Array used to initiliaze the return value
  \param endianess Target endianess for serialization
  \return A unaligned_data object which content is equal to raw_data
*/
template<size_t N>
unaligned_data<N> make_unaligned_data(const byte_t (&raw_data)[N], system::endianess endianess) {
  return unaligned_data<N>{raw_data, endianess};
}

/*!
  \brief Unaligned storage with fixed target types
  \details
    The object holds values of provided type in an unaligned maners (ie, there is no padding between two consecutive
    values).
  \tparam Ts... Types of the serialized values
*/
template<typename... Ts>
class unaligned_tuple : public unaligned_data<ctm::sum(sizeof(Ts)...)> {
  constexpr static auto list = ctm::typelist<Ts...>{};
  constexpr static auto size = ctm::sum(sizeof(Ts)...);

public:
  //! \brief Type of one of the serialized values
  //! \tparam I Index of the requested value's type
  template<size_t I>
  using arg_t = ctm::grab<decltype(list.get(ctm::size_h<I>{}))>;

  /*!
    \brief Forward arguments to base's constructor
    \param endianess Target endianess for serialization
    \see unaligned_data::unaligned_data(system::endianess)
  */
  explicit unaligned_tuple(system::endianess endianess) : unaligned_data<size>{endianess} {}

  /*!
    \brief Serialize the provided values
    \tparam Args... Serialized values' types
    \param endianess Target endianess for serialization
    \param args... Values to be serialized
  */
  template<typename... Args, typename = ctm::enable_t<sizeof...(Args) == sizeof...(Ts)>>
  explicit unaligned_tuple(system::endianess endianess, const Args&... args) : unaligned_data<size>{endianess} {
    lay(ctm::srange<0, sizeof...(Ts)>{}, args...);
  }

  /*!
    \brief Unserialize one of the value held by the object
    \tparam I Index of the requested value
    \return A copy of the serialized value or an array_wrapper if I designate an array type
  */
#ifdef DOXYGEN
  template<size_t I> auto get() const;
#else
  template<size_t I>
  decltype(ctm::declval<unaligned_data<size>>().template interpret_as<arg_t<I>>(0))
  get() const {
    constexpr auto offset = ctm::slist<sizeof(Ts)...>{}
      .take(ctm::size_h<I>{})
      .accumulate(0, ctm::sum<size_t, size_t>);

    return unaligned_data<size>::template interpret_as<arg_t<I>>(offset);
  }
#endif

  /*!
    \brief Set one of the value held by the object
    \tparam I Index of the value which will be set
    \param value Value to be copied from
  */
  template<size_t I>
  void set(const arg_t<I>& value) {
    constexpr auto offset = ctm::slist<sizeof(Ts)...>{}
      .take(ctm::size_h<I>{})
      .accumulate(0, ctm::sum<size_t, size_t>);

    unaligned_data<size>::write(value, offset);
  }

private:
  template<size_t... Is, typename... Args>
  void lay(ctm::size_h<Is...>, const Args&... args) {
     // TODO : à changer pour quelque chose de plus propre
     using discard = int[];
     discard {0, (set<Is>(args), 0)...};
  }

};

/*!
  \brief Construct an unaligned_tuple object provided constant lvalue to values
  \tparam Args... Deduced types of the provided values.
  \param endianess Target endianess for serialization
  \param args... Values to be serialized into the return value
  \return unaligned_tuple object holding a serialized copy of the provided values.
*/
template<typename... Args>
unaligned_tuple<Args...> make_unaligned_arguments(system::endianess endianess, const Args&... args) {
  return unaligned_tuple<Args...>{endianess, args...};
}

} // namespace upd
