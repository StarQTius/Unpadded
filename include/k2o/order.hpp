//! \file
//! \brief Order execution at receiver side

#pragma once

#include <upd/tuple.hpp>
#include <upd/type.hpp>

#include "detail/function_reference.hpp"
#include "detail/input_tuple.hpp"
#include "detail/normalize_to_tuple.hpp"
#include "detail/sfinae.hpp"
#include "detail/signature.hpp"
#include "detail/unique_ptr.hpp"
#include "status.hpp"

#include "detail/def.hpp"

namespace k2o {
namespace detail {

//! \brief Type alias used when a functor acting as a input byte stream is needed
using src_t = abstract_function<upd::byte_t()>;

//! \brief Type alias used when a functor acting as a output byte stream is needed
using dest_t = abstract_function<void(upd::byte_t)>;

//! \brief Serialize a single integer value as a sequence of byte then map a functor over this sequence
//! \tparam Endianess considered endianess when serializing the value
//! \tparam Signed_Mode considered signed representation when serializing the value
//! \param insert_byte functor to be mapped over the byte sequence
//! \param value value to be serialized
//! \return A status code
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename T, sfinae::require_not_tuple<T> = 0>
status insert(dest_t &insert_byte, const T &value) {
  using namespace upd;

  auto output = make_tuple<Endianess, Signed_Mode>(value);
  for (upd::byte_t byte : output)
    insert_byte(byte);

  return status::OK;
}

//! \brief Call a functor with arguments serialized as a byte sequence
//! \details
//! \tparam Tuple instance of 'upd::tuple' class template which will be used to hold the byte sequence
//! \param fetch_byte functor which will enumerate the byte sequence
//! \param ftor functor which will be called with the arguments
//! \return A status code
template<typename Tuple, typename F>
status call(src_t &fetch_byte, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args)
    byte = fetch_byte();
  input_args.invoke(FWD(ftor));

  return status::OK;
}

//! \brief Call a functor with arguments serialized as a byte sequence and serialize the result of that call
//! \tparam Tuple instance of the 'upd::tuple' class template which will be used to hold the byte sequence
//! \param fetch_byte functor which will enumerate the byte sequence
//! \param ftor functor which will be called with the arguments
//! \return A status code
#ifdef DOXYGEN
template<typename Tuple, typename F>
status call(src_t &fetch_byte, dest_t &, F &&ftor);
#endif

template<typename Tuple, typename F, sfinae::require_is_void<detail::return_t<F>> = 0>
status call(src_t &fetch_byte, dest_t &, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args)
    byte = fetch_byte();
  input_args.invoke(FWD(ftor));

  return status::OK;
}

template<typename Tuple, typename F, sfinae::require_not_void<detail::return_t<F>> = 0>
status call(src_t &fetch_byte, dest_t &insert_byte, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args)
    byte = fetch_byte();

  return insert<Tuple::storage_endianess, Tuple::storage_signed_mode>(insert_byte, input_args.invoke(FWD(ftor)));
}

//! \brief Implementation of the 'order' class behaviour
//! \details
//!   This class holds the functor passed to the 'order' constructor and is used to deduce the appropriate 'upd::tuple'
//!   template instance for holding the functor parameters.
//! \tparam F type of the held functor
//! \tparam Endianess 'Endianess' template parameter of the 'upd::tuple' template instance
//! \tparam Signed_Mode 'Signed_Mode' template parameter of the 'upd::tuple' template instance
template<typename F, upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct order_model_impl {
  using tuple_t = input_tuple<Endianess, Signed_Mode, F>;

  explicit order_model_impl(F &&ftor) : ftor{FWD(ftor)} {}

  F ftor;
};

//! \brief Abstract class used for setting up type erasure in the 'order' class
//! \details
//!   This class is base of 'order_model' as a the "Concept" class in the type erasure pattern.
struct order_concept {
  virtual ~order_concept() = default;
  virtual status operator()(src_t &&) = 0;
  virtual status operator()(src_t &&, dest_t &&) = 0;

  size_t input_size;
  size_t output_size;
};

//! \brief Derived class used for setting up type erasure in the 'order' class
//! \details
//!   This class is derived from the 'order_concept' as a the "Model" class in the type erasure pattern.
//!   This implementation of 'order_concept' uses the 'src_t' and 'dest_t' functors to fetch the arguments for calling
//!   the functor as a byte sequence and serialize the expression resulting from that call.
//! \tparam F type of the held functor
//! \tparam Endianess considered endianess when serializing/unserializing arguments
//! \tparam Signed_Mode considered signed mode when serializing/unserializing arguments
template<typename F, upd::endianess Endianess, upd::signed_mode Signed_Mode>
class order_model : public order_concept {
  using impl_t = order_model_impl<F, Endianess, Signed_Mode>;
  using tuple_t = typename impl_t::tuple_t;

public:
  explicit order_model(F &&ftor) : m_impl{FWD(ftor)} {
    order_model::input_size = tuple_t::size;
    order_model::output_size = normalize_to_tuple_t<Endianess, Signed_Mode, return_t<F>>::size;
  }

  status operator()(src_t &&fetch_byte) final { return detail::call<tuple_t>(fetch_byte, FWD(m_impl.ftor)); }
  status operator()(src_t &&fetch_byte, dest_t &&insert_byte) final {
    return detail::call<tuple_t>(fetch_byte, insert_byte, FWD(m_impl.ftor));
  }

private:
  impl_t m_impl;
};

} // namespace detail

//! \brief Functor wrapper working as a arguments and return values serializer / unserializer
//! \details
//!   This class holds a functor member variable. It acts as a functor itself when provided one or two functors. Those
//!   provided functor must act as input / output byte stream. The resultings byte sequences represents the arguments or
//!   return values associated with the calls of the held functor. The values may only be of integer type (signed or
//!   unsigned) array of integers or 'upd::tuple' objects.
class order {
public:
  //! \brief Wrap a copy of the provided functor
  //! \tparam Endianess considered endianess when serializing / unserializing values
  //! \tparam Signed_Mode considered signed integer representation when serializing / unserializing values
  //! \param ftor functor to be wrapped
  template<upd::endianess Endianess = upd::endianess::BUILTIN,
           upd::signed_mode Signed_Mode = upd::signed_mode::BUILTIN,
           typename F>
  explicit order(F &&ftor, upd::endianess_h<Endianess> = {}, upd::signed_mode_h<Signed_Mode> = {})
      : m_concept_uptr{new detail::order_model<F, Endianess, Signed_Mode>{FWD(ftor)}} {}

  //! \brief Call the held functor with the serialized argument delivered by the provided input byte stream
  //! \details
  //!   The input byte stream is provided through the functor passed as parameter. When called with no parameters, it
  //!   must return a 'upd::byte_t' value. The necessary arguments for the held functor call are unserialized from the
  //!   byte sequence obtained from the input byte stream. If any, the return value resulting from the call is
  //!   discarded.
  //! \param fetch_byte Functor acting as a input byte stream
  template<typename Src_F>
  status operator()(Src_F &&fetch_byte) {
    return (*m_concept_uptr)(detail::make_function_reference(fetch_byte));
  }

  //! \brief Call the held functor and serialize / unserialize necessary value with the provided byte stream
  //! \details
  //!   This function must be provided an input byte stream and an output byte stream. The input byte stream is
  //!   provided through the functor passed as first parameter. When called with no parameters, it must return a
  //!   'upd::byte_t' value. The output byte stream is provided through the functor passed as second parameter. It
  //!   should be callable with a 'upd::byte_t' value. The necessary arguments for the held functor call are
  //!   unserialized from the byte sequence obtained from the input byte stream. If any, the return value resulting from
  //!   the call is serialized then inserted into the output byte stream.
  //! \param fetch_byte Functor acting as a input byte stream
  //! \param fetch_byte Functor acting as a input byte stream
  template<typename Src_F, typename Dest_F>
  status operator()(Src_F &&fetch_byte, Dest_F &&insert_byte) {
    return (*m_concept_uptr)(detail::make_function_reference(fetch_byte), detail::make_function_reference(insert_byte));
  }

  //! \brief Get the size in bytes of the payload needed to call the wrapped functor
  //! \return The size of the payload in bytes
  size_t input_size() const { return m_concept_uptr->input_size; }

  //! \brief Get the size in bytes of the payload representing the returned value from a call to the wrapped functor
  //! \return The size of the payload in bytes
  size_t output_size() const { return m_concept_uptr->output_size; }

private:
  detail::unique_ptr<detail::order_concept> m_concept_uptr;
};

} // namespace k2o

#include "detail/undef.hpp"
