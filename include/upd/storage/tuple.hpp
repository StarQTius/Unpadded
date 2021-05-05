#pragma once

#include "ct_magic.hpp"

#include "upd/format.hpp"
#include "upd/type.hpp"

#include "unaligned_data.hpp"

/*!
  \file
  \brief Definition of the tuple class
*/

namespace upd {

/*!
  \brief Unaligned storage with fixed target types
  \details
    The object holds values of provided type in an unaligned maners (ie, there is no padding between two consecutive
    values).
  \tparam Ts... Types of the serialized values
*/
template<typename... Ts>
class tuple {
  constexpr static auto list = ctm::typelist<Ts...>{};

public:
  //! \brief Type of one of the serialized values
  //! \tparam I Index of the requested value's type
  template<size_t I>
  using arg_t = ctm::grab<decltype(list.get(ctm::size_h<I>{}))>;

  //! \brief Storage size in byte
  constexpr static auto size = ctm::sum(sizeof(Ts)...);

  /*!
    \brief Default initialize the object content
    \param endianess Target endianess for serialization
  */
  explicit tuple(endianess data_endianess, signed_mode data_signed_mode) :
    m_storage{data_endianess, data_signed_mode} {}

  /*!
    \brief Serialize the provided values
    \tparam Args... Serialized values' types
    \param data_endianess Target endianess for serialization
    \param data_signed_mode Target signed representation for serialization
    \param args... Values to be serialized
  */
  template<typename... Args, typename = ctm::enable_t<sizeof...(Args) == sizeof...(Ts)>>
  explicit tuple(
    endianess data_endianess,
    signed_mode data_signed_mode,
    const Args&... args) :
    m_storage{data_endianess, data_signed_mode}
  {
    lay(ctm::srange<0, sizeof...(Ts)>{}, args...);
  }

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
  decltype(ctm::declval<unaligned_data<size>>().template interpret_as<arg_t<I>>(0))
  get() const {
    constexpr auto offset = ctm::slist<sizeof(Ts)...>{}
      .take(ctm::size_h<I>{})
      .accumulate(0, ctm::sum<size_t, size_t>);

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
    constexpr auto offset = ctm::slist<sizeof(Ts)...>{}
      .take(ctm::size_h<I>{})
      .accumulate(0, ctm::sum<size_t, size_t>);

    m_storage.write(value, offset);
  }

private:
  template<size_t... Is, typename... Args>
  void lay(ctm::size_h<Is...>, const Args&... args) {
     // TODO : à changer pour quelque chose de plus propre
     using discard = int[];
     discard {0, (set<Is>(args), 0)...};
  }

  unaligned_data<ctm::sum(sizeof(Ts)...)> m_storage;

};

/*!
  \brief Construct a tuple object provided constant lvalue to values
  \tparam Args... Deduced types of the provided values.
  \param data_endianess Target endianess for serialization
  \param data_signed_mode Target signed representation for serialization
  \param args... Values to be serialized into the return value
  \return tuple object holding a serialized copy of the provided values.
*/
template<typename... Args>
tuple<Args...> make_tuple(
  endianess data_endianess,
  signed_mode data_signed_mode,
  const Args&... args)
{
  return tuple<Args...>{data_endianess, data_signed_mode, args...};
}

} // namespace upd
