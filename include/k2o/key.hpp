//! \file
//! \brief Order execution at sender side with indexing

#pragma once

#include <cstdint>

#include <boost/type_traits/remove_cv_ref.hpp>
#include <upd/format.hpp>

#include "detail/signature.hpp"
#include "key_base.hpp"

namespace k2o {

template<uint16_t Index,
         typename F,
         upd::endianess Endianess = upd::endianess::BUILTIN,
         upd::signed_mode Signed_Mode = upd::signed_mode::BUILTIN>
class key : public key<Index, detail::signature_t<F>, Endianess, Signed_Mode> {};

//! \brief Serializing / unserializing and indexing helper class according to a function signature
//! \details
//!   When serializing, an index is appended at the beginning of the payload. It can be used by the slave to determine
//!   which order to execute. However, no check are performed when unserializing as 'key' objects do not expect an
//!   index to be appended.
//! \tparam Index Order index
//! \tparam R return type associated with the signature
//! \tparam Args... argument types associated with the signature
//! \tparam Endianess considered endianess when serializing/unserializing arguments
//! \tparam Signed_Mode considered signed integer representation when serializing/unserializing arguments
template<uint16_t Index, typename R, typename... Args, upd::endianess Endianess, upd::signed_mode Signed_Mode>
class key<Index, R(Args...), Endianess, Signed_Mode> : public key_base<R(Args...), Endianess, Signed_Mode> {
public:
  constexpr static auto index = Index;
  detail::
      serialized_message<upd::endianess::BUILTIN, upd::signed_mode::BUILTIN, uint16_t, boost::remove_cv_ref_t<Args>...>
      operator()(const Args &...args) const {
    return {Index, args...};
  }
};

} // namespace k2o
