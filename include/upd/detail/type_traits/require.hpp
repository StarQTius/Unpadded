//! \file

#pragma once

#include <iterator>
#include <type_traits>

#include "../../format.hpp"
#include "../../type.hpp"
#include "is_array.hpp"
#include "is_key.hpp"
#include "is_keyring.hpp"
#include "is_tuple.hpp"
#include "is_user_serializable.hpp"
#include "signature.hpp"
#include "typelist.hpp"

#if !defined(DOXYGEN)

#define UPD_REQUIRE(...) ::upd::detail::require<__VA_ARGS__> = 0
#define UPD_REQUIREMENT(NAME, ...) ::upd::detail::require_##NAME<__VA_ARGS__> = 0
#define UPD_REQUIRE_CLASS(...)                                                                                         \
  bool __Require_Class = false, ::upd::detail::require < __Require_Class || (__VA_ARGS__) > = 0

#else // !defined(DOXYGEN)

#define UPD_REQUIRE(...)
#define UPD_REQUIREMENT(...)
#define UPD_REQUIRE_CLASS(...)

#endif // !defined(DOXYGEN)

namespace upd {

template<endianess Endianess, signed_mode Signed_Mode, typename... Ts>
class tuple; // IWYU pragma: keep

namespace detail {

//! \brief Require the provided expression to be true
template<bool Expression, typename U = int>
using require = typename std::enable_if<Expression, U>::type;

//! \brief Require the provided types to be well-formed
template<typename U, typename... Args>
using require_t = decltype(tlist_t<Args...>{}, U{});

//! \brief Require the provided type to be a template instance of `tuple`
template<typename T, typename U = int>
using require_is_tuple = require<is_tuple<T>::value, U>;

//! \brief Require the provided type not to be a template instance of `tuple`
template<typename T, typename U = int>
using require_not_tuple = require<!is_tuple<T>::value, U>;

//! \brief Require the provided type to be `void`
template<typename T, typename U = int>
using require_is_void = require<std::is_void<T>::value, U>;

//! \brief Require the provided type not to be `void`
template<typename T, typename U = int>
using require_not_void = require<!std::is_void<T>::value, U>;

//! \brief Require the provided type to be a function type
template<typename T, typename U = int>
using require_is_function =
    require<std::is_function<typename std::remove_pointer<typename std::decay<T>::type>::type>::value, U>;

//! \brief Require the provided type to be a valid keyring
template<typename T, typename U = int>
using require_is_keyring = require<is_keyring<T>::value, U>;

//! \brief Require the instances of the given type to be invocable on no parameters and return a byte
template<typename F, typename U = int>
using require_input_invocable = require<has_signature<F, byte_t()>::value, U>;

//! \brief Require the instances of the given type to be invocable on a byte
template<typename F, typename U = int>
using require_output_invocable = require<has_signature<F, void(byte_t)>::value, U>;

//! \brief Require the given type to be an input iterator to a byte sequence
template<typename T, typename U = int>
using require_input_byte_iterator =
    require_t<U,
              decltype(std::input_iterator_tag{typename std::iterator_traits<T>::iterator_category{}}),
              decltype(std::declval<byte_t &>() = *std::declval<T &>()++)>;

//! \brief Require the given type to be an output iterator which allows to write byte sequences
template<typename T, typename U = int>
using require_output_byte_iterator =
    require_t<U, typename std::iterator_traits<T>::iterator_category, decltype(*std::declval<T &>()++ = byte_t{})>;

//! \brief Require `T` to be invocable
template<typename T, typename U = int>
using require_invocable = require<is_invocable<T>::value>;

//! \brief Require the provided type to be an unsigned integer type
template<typename T, typename U = int>
using require_unsigned_integer = require<std::is_unsigned<T>::value, U>;

//! \brief Require the provided type to be a signed integer type
template<typename T, typename U = int>
using require_signed_integer = require<std::is_signed<T>::value, U>;

//! \brief Require the provided type to be an bounded array type
template<typename T, typename U = int>
using require_array = require<detail::is_array<T>::value, U>;

//! \brief Require the provided type not to be an bounded array type
template<typename T, typename U = int>
using require_not_array = require<!detail::is_array<T>::value, U>;

//! \brief Require the provided type not to be a pointer type
template<typename T, typename U = int>
using require_not_pointer = require<!std::is_pointer<T>::value, U>;

//! \brief Require the provided pack not to be empty
template<typename... Ts>
using require_not_empty_pack = require<sizeof...(Ts) != 0, int>;

//! \brief Require the provided type to have an user-defined extension
template<typename T, typename U = int>
using require_is_user_serializable = require<is_user_serializable<T>::value, U>;

//! \brief Require `T` to be a valid key
template<typename T, typename U = int>
using require_key = require<is_key<T>::value, U>;

} // namespace detail
} // namespace upd
