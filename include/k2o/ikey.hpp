//! \file
//! \brief Order execution at sender side with indexing

#pragma once

#include <k2o/detail/signature.hpp>
#include <k2o/key.hpp>

namespace k2o {
namespace detail {

//! \brief Helper class that allows an 'ikey' object to introduce endianess and signed integer representation
//! \tparam Endianess endianess specified by the 'ikey' object
//! \tparam Signed_Mode signed integer representation specified by the 'ikey' object
//! \tparam R if unserializing, type of the result
//! \tparam Args... if serializing, type of the parameters
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, size_t Index, typename R, typename... Args>
struct serialized_message_maker_with_index {
  detail::serialized_message<Endianess, Signed_Mode, size_t, boost::remove_cv_ref_t<Args>...>
  operator()(const Args &... args) const {
    return {Index, args...};
  }
};

} // namespace detail

template<size_t Index, typename F>
class ikey : ikey<Index, detail::signature_t<F>> {};

//! \brief Serializing / unserializing and indexing helper class according to a function signature
//! \detail
//!   When serializing, an index is appended at the beginning of the payload. It can be used by the slave to determine
//!   which order to execute. However, no check are performed when unserializing as 'ikey' objects do not expect an
//!   index to be appended.
//! \tparam R return type associated with the signature
//! \tparam Args... argument types associated with the signature
//! \tparam Endianess considered endianess when serializing/unserializing arguments
//! \tparam Signed_Mode considered signed integer representation when serializing/unserializing arguments
template<size_t Index, typename R, typename... Args>
class ikey<Index, R(Args...)> : key<R(Args...)> {
public:
  constexpr static auto index = Index;

  template<upd::endianess Endianess, upd::signed_mode Signed_Mode>
  detail::serialized_message_maker_with_index<Endianess, Signed_Mode, Index, R, Args...>
  operator[](profile<Endianess, Signed_Mode>) {
    return {};
  }

  detail::
      serialized_message<upd::endianess::BUILTIN, upd::signed_mode::BUILTIN, size_t, boost::remove_cv_ref_t<Args>...>
      operator()(const Args &... args) const {
    return {Index, args...};
  }
};

} // namespace k2o
