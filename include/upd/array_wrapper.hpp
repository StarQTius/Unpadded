#pragma once

#include <cstring>

#include "type.hpp"

/*!
  \file
  \brief Definition of the array_wrapper class
*/

namespace upd {

/*!
  \brief Template class not meant to be instantiated
  \details It allows to introduce the partial specialization array_wrapper<T[N]>
*/
template<typename T>
struct array_wrapper;

/*!
  \brief Utility class wrapping an C-style array, analogously to std::array
  \details This is a partial specialization of array_wrapper.
  \tparam T Type of the element of the contained array
  \tparam N Size of the contained array
*/
template<typename T, size_t N>
struct array_wrapper<T[N]> {
  //! \brief Type of the contained elements
  using type = T;
  using array_t = T[N];

  //! \brief Size of the container
  constexpr static auto size = N;

  /*!
    \brief Initialize the object's content of the object
  */
  array_wrapper() : content{} {}

  /*!
    \brief Copy from a buffer to the object's content of the object
    \param ptr Pointer to the buffer
  */
  explicit array_wrapper(const T *ptr) { memcpy(content, ptr, N); }

  /*!
    \name Array-like operators
    @{
  */

  T &operator[](size_t i) { return content[i]; }
  const T &operator[](size_t i) const { return content[i]; }

  T &operator*() { return *content; }
  const T &operator*() const { return *content; }

  T *operator->() { return content; }
  const T *operator->() const { return content; }

  T *operator+(size_t n) { return content + n; }
  const T *operator+(size_t n) const { return content + n; }

  operator array_t &() { return content; }
  operator const array_t &() const { return content; }

  //! @}

  /*!
    \name Iterability
    @{
  */

  T *begin() { return content; }
  T *end() { return content + N; }
  const T *begin() const { return content; }
  const T *end() const { return content + N; }

  //! @}

  //! \brief Object's content
  //! \details Every operation on the object has the same result on the object's content.
  T content[N];
};

} // namespace upd
