//! \file
//! \brief Order execution at sender side

#pragma once

#include <boost/type_traits/remove_cv_ref.hpp>
#include <k2o/detail/function_reference.hpp>
#include <upd/format.hpp>
#include <upd/tuple.hpp>

#include "detail/any_function.hpp"
#include "detail/io.hpp"
#include "detail/sfinae.hpp"
#include "detail/signature.hpp"

#include "detail/def.hpp"

namespace k2o {
namespace detail {

//! \brief Simple wrapper around 'upd::tuple' whose content can be forwarded to a functor as a byte sequence
//! \details
//!   The content can be forwarded with the 'operator>>' function member. 'detail::serialized_message' object
//!   cannot be copied from to avoid unintentional copy.
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename... Ts>
struct serialized_message : detail::immediate_writer<serialized_message<Endianess, Signed_Mode, Ts...>> {
  //! \brief Store the payload
  serialized_message(const Ts &...values) : content{values...} {}

  serialized_message(const serialized_message &) = delete;
  serialized_message(serialized_message &&) = default;

  serialized_message &operator=(const serialized_message &) = delete;
  serialized_message &operator=(serialized_message &&) = default;

  using detail::immediate_writer<serialized_message<Endianess, Signed_Mode, Ts...>>::write_all;

  //! \brief Completely output the payload represented by the key
  template<typename Dest_F, REQUIREMENT(output_ftor, Dest_F)>
  void write_all(Dest_F &&insert_byte) const {
    for (auto byte : content)
      insert_byte(byte);
  }

  upd::tuple<Endianess, Signed_Mode, Ts...> content;
};

} // namespace detail

//! \brief Stores a callback to be called on the response from a device
//! \details This class is non-templated, therefore it is suitable for storage.
class key_with_hook {
  template<typename, upd::endianess, upd::signed_mode>
  friend class key_base;

public:
  //! \brief Call the stored callback on the provided parameters
  //! \param input_ftor Input functor the parameters will be extracted from
  template<typename F, REQUIREMENT(input_ftor, F)>
  void operator()(F &&input_ftor) const {
    m_restorer(m_callback_ptr, detail::make_function_reference<F>(input_ftor));
  }

  //! \copybrief operator()
  //! \param it Start of the range the parameters will be extracted from
  //! \return The error code resulting from the call to 'read_headerless_packet'
  template<typename It, REQUIREMENT(byte_iterator, It)>
  void operator()(It it) const {
    operator()([&]() { return *it++; });
  }

private:
  //! \brief Store a functor convertible to function pointer
  template<typename F, typename Key>
  explicit key_with_hook(F &&ftor, Key) : key_with_hook{+ftor, Key{}} {}

  //! \brief Store a function pointer as callback and its restorer
  template<typename R, typename... Args, typename Key>
  explicit key_with_hook(R (*f_ptr)(Args...), Key)
      : m_callback_ptr{reinterpret_cast<detail::any_function_t *>(f_ptr)},
        m_restorer{detail::restore_and_call<R(Args...), Key>} {}

  detail::any_function_t *m_callback_ptr;
  detail::restorer_t *m_restorer;
};

template<typename F,
         upd::endianess Endianess = upd::endianess::BUILTIN,
         upd::signed_mode Signed_Mode = upd::signed_mode::BUILTIN>
class key_base : public key_base<detail::signature_t<F>, Endianess, Signed_Mode> {};

//! \brief Serializing / unserializing helper class according to a function signature
//! \tparam R return type associated with the signature
//! \tparam Args... argument types associated with the signature
//! \tparam Endianess considered endianess when serializing/unserializing arguments
//! \tparam Signed_Mode considered signed integer representation when serializing/unserializing arguments
template<typename R, typename... Args, upd::endianess Endianess, upd::signed_mode Signed_Mode>
class key_base<R(Args...), Endianess, Signed_Mode>
    : public detail::immediate_reader<key_base<R(Args...), Endianess, Signed_Mode>, R> {
public:
  //! \brief Serialize arguments and prepare them for sending
  //! \details
  //!   This allows the following syntax : 'key_base(x1, x2, x3, ...) >> dest_f' (with 'dest_f' being a functor of
  //!   signature 'void(byte_t)'). 'dest_f' is called with every byte representing the data passed as parameter.
  //! \param args... arguments to be serialized
  //! \return A temporary object allowing the syntax mentioned above
#ifdef DOXYGEN
  auto operator()(const Args &...args) const;
#else
  detail::serialized_message<Endianess, Signed_Mode, boost::remove_cv_ref_t<Args>...>
  operator()(const Args &...args) const {
    return {args...};
  }
#endif

  //! \brief Unserialize a value from an input byte stream
  //! \details
  //!   The input byte stream is provided through the functor passed as parameter. When called with no parameters, it
  //!   must return a 'byte_t' value.
  //! \param fetch_byte Functor acting as a input byte stream
  //! \return the unserialized value
  template<typename Src_F>
  R read_all(Src_F &&fetch_byte) const {
    upd::tuple<Endianess, Signed_Mode, R> retval;
    for (auto &byte : retval)
      byte = FWD(fetch_byte)();

    return retval.template get<0>();
  }

  //! \brief Hook a callback to the key to be invoked when the order is done
  //! \param hook Callback to hook
  //! \return a 'key_with_hook' instance holding the provided hook
  template<typename F>
  key_with_hook with_hook(F &&hook) {
    return key_with_hook{FWD(hook), *this};
  }
};

#if __cplusplus >= 201703L
//! \brief (C++17) Create a 'key_base' object
//! \details
//!   This function use C++17 placeholder auto with template non-type parameter to allow nicer syntax.
//! \tparam Function function whose signature will be passed to the returned object
//! \return 'key_base<boost::remove_cv_ref_t<decltype(*Function)>>{}'
template<auto &Function,
         upd::endianess Endianess = upd::endianess::BUILTIN,
         upd::signed_mode Signed_Mode = upd::signed_mode::BUILTIN>
constexpr auto make_key(upd::endianess_h<Endianess> = {}, upd::signed_mode_h<Signed_Mode> = {}) {
  return key_base<boost::remove_cv_ref_t<decltype(Function)>, Endianess, Signed_Mode>{};
}
#endif // __cplusplus >= 201703L

} // namespace k2o

#include "detail/undef.hpp" // IWYU pragma: keep
