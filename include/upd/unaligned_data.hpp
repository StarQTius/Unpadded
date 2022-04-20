//! \file

#pragma once

#include <cstddef>
#include <cstring>
#include <type_traits>

#include "format.hpp"
#include "serialization.hpp"
#include "type.hpp"

namespace upd {

//! \brief Unaligned storage enabling reading and writing at any offset
//! \details
//!   The class holds an array of bytes used to store the serialized representation of values without padding due
//!   to memory alignment.
//!   The only reasonably serializable value are integer values; Thus the implementation of unaligned_data only support
//!   unsigned integer values. It doesn't handle signed representation yet.
//!   The user shall provide the target endianess so that the unsigned integer are serialized without depending on the
//!   platform endianess.
//! \tparam N Size of the content in bytes
//! \tparam Endianess endianess of the stored data
//! \tparam Signed_Mode signed mode of the stored data
template<std::size_t N, endianess Endianess = endianess::BUILTIN, signed_mode Signed_Mode = signed_mode::BUILTIN>
class unaligned_data {
public:
  //! \brief Storage size in byte
  constexpr static auto size = N;

  //! \brief Equals the endianess given as template parameter
  constexpr static auto storage_endianess = Endianess;

  //! \brief Equals the signed mode given as template parameter
  constexpr static auto storage_signed_mode = Signed_Mode;

  //! \brief Default initialize the underlying storage
  unaligned_data() = default;

  //! \brief Initialize the object content of the object by copying from a buffer
  //! \details The Nth first bytes of the buffer are copied.
  //! \param raw_data Pointer to the buffer
  explicit unaligned_data(const byte_t *raw_data) { memcpy(m_raw_data, raw_data, N); }

  //! \name Iterability
  //! @{
  byte_t *begin() { return m_raw_data; }
  byte_t *end() { return m_raw_data + N; }
  const byte_t *begin() const { return m_raw_data; }
  const byte_t *end() const { return m_raw_data + N; }
  //! @}

  //! \brief Access the object content
  //! \details There is no bound check performed.
  //! \param i Index of the accessed byte
  byte_t &operator[](std::size_t i) { return m_raw_data[i]; }

  //! \brief Access the object content
  //! \details There is no bound check performed.
  //! \param i Index of the accessed byte
  const byte_t &operator[](std::size_t i) const { return m_raw_data[i]; }

  //! \brief Interpret a part of a object content as a value of the given type
  //! \details
  //!   There is no bound check performed.
  //! \tparam T Requested type
  //! \param offset Start of the part of the content to be interpreted
  //! \return A copy of the value represented by the content at the given offset
  template<typename T>
  auto read_as(std::size_t offset) const
      -> decltype(upd::read_as<T, Endianess, Signed_Mode>(std::declval<byte_t *>())) {
    return upd::read_as<T, Endianess, Signed_Mode>(m_raw_data + offset);
  }

  //! \brief Serialize a value into the object content
  //! \details
  //!   There is no bound check performed.
  //! \tparam T Serilized value's type
  //! \param value Value to be serialized
  //! \param offset Start of the part of the content to be written
  template<typename T>
  void write_as(const T &value, std::size_t offset) {
    upd::write_as<Endianess, Signed_Mode>(value, m_raw_data + offset);
  }

private:
  byte_t m_raw_data[N];
};

//! \brief Construct an unaligned_data object provided a lvalue to a bounded array
//! \tparam N Size of the bounded array
//! \tparam Endianess Target endianess for serialization
//! \tparam Signed_Mode Target signed mode for serialization
//! \param raw_data Array used to initiliaze the return value
//! \return A unaligned_data object which content is equal to raw_data
template<endianess Endianess = endianess::BUILTIN, signed_mode Signed_Mode = signed_mode::BUILTIN, std::size_t N>
unaligned_data<N, Endianess, Signed_Mode> make_unaligned_data(const byte_t (&raw_data)[N]) {
  return unaligned_data<N, Endianess, Signed_Mode>{raw_data};
}

//! \brief Interpret a part of the provided 'unaligned_data' object content as a value of the given type
//! \details
//!   There is no bound check performed.
//! \tparam T Requested type
//! \param input_unaligned_data 'unaligned_data' object to read from
//! \param offset Start of the part of the content to be interpreted
//! \return A copy of the value represented by the content at the given offset
template<typename T, std::size_t N, endianess Endianess, signed_mode Signed_Mode>
decltype(std::declval<unaligned_data<N, Endianess, Signed_Mode>>().template read_as<T>(0))
read_as(const unaligned_data<N, Endianess, Signed_Mode> &input_unaligned_data, std::size_t offset) {
  return input_unaligned_data.template read_as<T>(offset);
}

//! \brief Serialize a value into the provided 'unaligned_data' object content
//! \details
//!   There is no bound check performed.
//! \tparam T Serilized value's type
//! \param value Value to be serialized
//! \param input_unaligned_data 'unaligned_data' object to write into
//! \param offset Start of the part of the content to be written
template<typename T, std::size_t N, endianess Endianess, signed_mode Signed_Mode>
void write_as(const T &value, unaligned_data<N, Endianess, Signed_Mode> &input_unaligned_data, std::size_t offset) {
  return input_unaligned_data.template write_as(value, offset);
}

} // namespace upd
