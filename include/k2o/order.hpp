//! \file
//! \brief Order execution at receiver side

#pragma once

#include <cstddef>

#include <type_traits>
#include <upd/format.hpp>
#include <upd/tuple.hpp>
#include <upd/type.hpp>

#include "detail/function_reference.hpp"
#include "detail/input_tuple.hpp"
#include "detail/io.hpp"
#include "detail/normalize_to_tuple.hpp"
#include "detail/sfinae.hpp"
#include "detail/signature.hpp"
#include "detail/unique_ptr.hpp"

#include "detail/def.hpp"

// IWYU pragma: no_include "upd/detail/value_h.hpp"

namespace k2o {
namespace detail {

//! \brief Type alias used when a functor acting as a input byte stream is needed
using src_t = abstract_function<upd::byte_t()>;

//! \brief Type alias used when a functor acting as a output byte stream is needed
using dest_t = abstract_function<void(upd::byte_t)>;

//! \brief Serialize a single integer value as a sequence of byte then map a functor over this sequence
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename T, sfinae::require_not_tuple<T> = 0>
void insert(dest_t &dest, const T &value) {
  using namespace upd;

  auto output = make_tuple(endianess_h<Endianess>{}, signed_mode_h<Signed_Mode>{}, value);
  for (upd::byte_t byte : output)
    dest(byte);
}

//! \brief Call a functor with arguments serialized as a byte sequence
template<typename Tuple, typename F>
void call(src_t &src, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args)
    byte = src();
  input_args.invoke(FWD(ftor));
}

//! \copydoc call
template<typename Tuple, typename F, sfinae::require_is_void<detail::return_t<F>> = 0>
void call(src_t &src, dest_t &, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args)
    byte = src();
  input_args.invoke(FWD(ftor));
}

//! \copydoc call
template<typename Tuple, typename F, sfinae::require_not_void<detail::return_t<F>> = 0>
void call(src_t &src, dest_t &dest, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args)
    byte = src();

  return insert<Tuple::storage_endianess, Tuple::storage_signed_mode>(dest, input_args.invoke(FWD(ftor)));
}

//! \brief Implementation of the 'order' class behaviour
//! \details
//!   This class holds the functor passed to the 'order' constructor and is used to deduce the appropriate 'upd::tuple'
//!   template instance for holding the functor parameters.
template<typename F, upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct order_model_impl {
  using tuple_t = input_tuple<Endianess, Signed_Mode, F>;

  explicit order_model_impl(F &&ftor) : ftor{FWD(ftor)} {}

  F ftor;
};

//! \brief Abstract class used for setting up type erasure in the 'order' class
struct order_concept {
  virtual ~order_concept() = default;
  virtual void operator()(src_t &&) = 0;
  virtual void operator()(src_t &&, dest_t &&) = 0;

  size_t input_size;
  size_t output_size;
};

//! \brief Derived class used for setting up type erasure in the 'order' class
//! \details
//!   This class is derived from the 'order_concept' as a the "Model" class in the type erasure pattern.
//!   This implementation of 'order_concept' uses the 'src_t' and 'dest_t' functors to fetch the arguments for calling
//!   the functor as a byte sequence and serialize the expression resulting from that call.
template<typename F, upd::endianess Endianess, upd::signed_mode Signed_Mode>
class order_model : public order_concept {
  using impl_t = order_model_impl<F, Endianess, Signed_Mode>;
  using tuple_t = typename impl_t::tuple_t;

public:
  explicit order_model(F &&ftor) : m_impl{FWD(ftor)} {
    order_model::input_size = tuple_t::size;
    order_model::output_size = normalize_to_tuple_t<Endianess, Signed_Mode, return_t<F>>::size;
  }

  void operator()(src_t &&src) final { return detail::call<tuple_t>(src, FWD(m_impl.ftor)); }
  void operator()(src_t &&src, dest_t &&dest) final { return detail::call<tuple_t>(src, dest, FWD(m_impl.ftor)); }

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
class order : public detail::immediate_process<order, void> {
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
  //! \param src Functor acting as a input byte stream
  template<typename Src>
  void operator()(Src &&src) {
    operator()(FWD(src), [](upd::byte_t) {});
  }

  using detail::immediate_process<order, void>::operator();

  //! \brief Call the held functor and serialize / unserialize necessary value with the provided byte stream
  //! \details
  //!   This function must be provided an input byte stream and an output byte stream. The input byte stream is
  //!   provided through the functor passed as first parameter. When called with no parameters, it must return a
  //!   'upd::byte_t' value. The output byte stream is provided through the functor passed as second parameter. It
  //!   should be callable with a 'upd::byte_t' value. The necessary arguments for the held functor call are
  //!   unserialized from the byte sequence obtained from the input byte stream. If any, the return value resulting from
  //!   the call is serialized then inserted into the output byte stream.
  //! \param src Byte input functor
  //! \param dest Byte output functor
  template<typename Src, typename Dest, sfinae::require_input_ftor<Src> = 0, sfinae::require_output_ftor<Dest> = 0>
  void operator()(Src &&src, Dest &&dest) {
    return (*m_concept_uptr)(detail::make_function_reference(src), detail::make_function_reference(dest));
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

class no_storage_order : public detail::immediate_process<no_storage_order, void> {
public:
  using detail::immediate_process<no_storage_order, void>::operator();

  template<typename F, F Ftor, upd::endianess Endianess, upd::signed_mode Signed_Mode>
  explicit no_storage_order(std::integral_constant<F &, Ftor>,
                            upd::endianess_h<Endianess>,
                            upd::signed_mode_h<Signed_Mode>)
      : wrapper{+[](detail::src_t &&src, detail::dest_t &&dest) {
          detail::input_tuple<Endianess, Signed_Mode, F> parameters_tuple;
          for (auto &byte : parameters_tuple)
            byte = src();
          auto return_tuple = upd::make_tuple(
              upd::endianess_h<Endianess>{}, upd::signed_mode_h<Signed_Mode>{}, parameters_tuple.invoke(Ftor));
          for (auto byte : return_tuple)
            dest(byte);
        }} {}

  template<typename Src, typename Dest, sfinae::require_input_ftor<Src> = 0, sfinae::require_output_ftor<Dest> = 0>
  void operator()(Src &&src, Dest &&dest) {
    wrapper(detail::make_function_reference(src), detail::make_function_reference(dest));
  }

private:
  void (*wrapper)(detail::src_t &&, detail::dest_t &&);
};
} // namespace k2o

#include "detail/undef.hpp" // IWYU pragma: keep
