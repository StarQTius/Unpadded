#pragma once

#include "ct_magic.hpp"

#include "upd/format.hpp"
#include "upd/type.hpp"

#include "unaligned_data.hpp"

/*!
  \file
  \brief Definition of the unaligned_tuple class
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
    \see unaligned_data::unaligned_data(endianess)
  */
  explicit unaligned_tuple(endianess data_endianess, signed_mode data_signed_mode) :
    unaligned_data<size>{data_endianess, data_signed_mode} {}

  /*!
    \brief Serialize the provided values
    \tparam Args... Serialized values' types
    \param data_endianess Target endianess for serialization
    \param data_signed_mode Target signed representation for serialization
    \param args... Values to be serialized
  */
  template<typename... Args, typename = ctm::enable_t<sizeof...(Args) == sizeof...(Ts)>>
  explicit unaligned_tuple(
    endianess data_endianess,
    signed_mode data_signed_mode,
    const Args&... args) :
    unaligned_data<size>{data_endianess, data_signed_mode}
  {
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
  \param data_endianess Target endianess for serialization
  \param data_signed_mode Target signed representation for serialization
  \param args... Values to be serialized into the return value
  \return unaligned_tuple object holding a serialized copy of the provided values.
*/
template<typename... Args>
unaligned_tuple<Args...> make_unaligned_arguments(
  endianess data_endianess,
  signed_mode data_signed_mode,
  const Args&... args)
{
  return unaligned_tuple<Args...>{data_endianess, data_signed_mode, args...};
}

} // namespace upd
