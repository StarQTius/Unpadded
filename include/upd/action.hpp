//! \file

#pragma once

#include <cstddef>
#include <memory>

#include "format.hpp"
#include "tuple.hpp"
#include "type.hpp"
#include "unevaluated.hpp"
#include "upd.hpp"

#include "detail/function_reference.hpp"
#include "detail/io/immediate_process.hpp"
#include "detail/static_error.hpp"
#include "detail/type_traits/flatten_tuple.hpp"
#include "detail/type_traits/input_tuple.hpp"
#include "detail/type_traits/require.hpp"
#include "detail/type_traits/signature.hpp"

// IWYU pragma: no_include "upd/detail/value_h.hpp"

namespace upd {
namespace detail {

//! \brief Input invocable with erased type
using src_t = abstract_function<byte_t()>;

//! \brief Output invocable with erased type
using dest_t = abstract_function<void(byte_t)>;

//! \brief Serialize `value` as a sequence of byte then call `dest` on every byte of that sequence
template<endianess Endianess, signed_mode Signed_Mode, typename T, UPD_REQUIREMENT(not_tuple, T)>
void insert(dest_t &dest, const T &value) {
  using namespace upd;

  auto output = make_tuple(endianess_h<Endianess>{}, signed_mode_h<Signed_Mode>{}, value);
  for (byte_t byte : output)
    dest(byte);
}

//! \brief Invoke `ftor` on the unserialized arguments from `src` and write the serialized return value to `dest`
template<typename Tuple, typename F>
void call(src_t &src, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args)
    byte = src();
  input_args.invoke(FWD(ftor));
}

//! \copydoc call
template<typename Tuple, typename F, UPD_REQUIREMENT(is_void, detail::return_t<F>)>
void call(src_t &src, dest_t &, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args)
    byte = src();
  input_args.invoke(UPD_FWD(ftor));
}

//! \copydoc call
template<typename Tuple, typename F, UPD_REQUIREMENT(not_void, detail::return_t<F>)>
void call(src_t &src, dest_t &dest, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args)
    byte = src();

  return insert<Tuple::storage_endianess, Tuple::storage_signed_mode>(dest, input_args.invoke(UPD_FWD(ftor)));
}

//! \brief Implementation of the `action` class behaviour
//!
//! This class holds the functor passed to the `action` constructor and is used to deduce the appropriate `tuple`
//! template instance for holding the functor parameters.
template<typename F, endianess Endianess, signed_mode Signed_Mode>
struct action_model_impl {
  using tuple_t = input_tuple<Endianess, Signed_Mode, F>;

  explicit action_model_impl(F &&ftor) : ftor{UPD_FWD(ftor)} {}

  F ftor;
};

//! \brief Abstract class used for setting up type erasure in the `action` class
struct action_concept {
  virtual ~action_concept() = default;
  virtual void operator()(src_t &&, dest_t &&) = 0;

  std::size_t input_size;
  std::size_t output_size;
};

//! \brief Derived class used for setting up type erasure in the `action` class
//!
//! This class is derived from the `action_concept` structure and act as the "Model" class in the type erasure pattern.
//! This class along with the `action_concept` class are internals and meant only to be used within the `action` class
//! from the public API. When an `action` instance is called, the underlying `action_model` instance calls the managed
//! callback. The input and output byte streams which the parameters and the return value will be read from or written
//! to are wrapped into `abstract_function` instances (because virtual functions cannot be templated). Do note however
//! that unlike `std::function`, constructing `abstract_function` instances do not make use of dynamic allocation and so
//! does an `action` instance call.
template<typename F, endianess Endianess, signed_mode Signed_Mode>
class action_model : public action_concept {
  using impl_t = action_model_impl<F, Endianess, Signed_Mode>;
  using tuple_t = typename impl_t::tuple_t;

public:
  explicit action_model(F &&ftor) : m_impl{UPD_FWD(ftor)} {
    action_model::input_size = tuple_t::size;
    action_model::output_size = flatten_tuple_t<Endianess, Signed_Mode, return_t<F>>::size;
  }

  void operator()(src_t &&src, dest_t &&dest) final { return detail::call<tuple_t>(src, dest, UPD_FWD(m_impl.ftor)); }

private:
  impl_t m_impl;
};

//! @{
//! \brief Wrap a callback with static storage duration or a free function into another free function

//! This overload accepts any callback returning `void`.
template<endianess Endianess, signed_mode Signed_Mode, typename F, F Ftor, UPD_REQUIREMENT(is_void, return_t<F>)>
void static_storage_duration_callback_wrapper(src_t &&src, dest_t &&dest) {
  input_tuple<Endianess, Signed_Mode, F> parameters_tuple;
  for (auto &byte : parameters_tuple)
    byte = src();
  parameters_tuple.invoke(Ftor);
}

//! This overload accepts any callback not returning `void`.
template<endianess Endianess, signed_mode Signed_Mode, typename F, F Ftor, UPD_REQUIREMENT(not_void, return_t<F>)>
void static_storage_duration_callback_wrapper(src_t &&src, dest_t &&dest) {
  input_tuple<Endianess, Signed_Mode, F> parameters_tuple;
  for (auto &byte : parameters_tuple)
    byte = src();
  auto return_tuple = make_tuple(endianess_h<Endianess>{}, signed_mode_h<Signed_Mode>{}, parameters_tuple.invoke(Ftor));
  for (auto byte : return_tuple)
    dest(byte);
}

//! @}

} // namespace detail

//! \brief Wrapper around a callback which serialize / unserialize parameters and return values
//!
//! Given a byte sequence generated by a `key` instance, an `action` instance (whose underlying callback has the same
//! signature as the aforesaid `key` instance) is able to unserialize the parameters from that byte sequence, invoke the
//! underlying callback and serialize the return value as a byte sequence (which can later be unserialized by the same
//! `key` instance to obtain the return value).
class action : public detail::immediate_process<action, void> {
public:
  action() = default;

  //! @{
  //! \brief Wrap a copy of a provided callback

  //! \tparam Endianess, Signed_Mode Serialization parameters
  //! \param ftor Callback to be wrapped
  template<endianess Endianess, signed_mode Signed_Mode, typename F, UPD_REQUIREMENT(invocable, F)>
  explicit action(F &&ftor, endianess_h<Endianess>, signed_mode_h<Signed_Mode>)
      : m_concept_uptr{new detail::action_model<F, Endianess, Signed_Mode>{UPD_FWD(ftor)}} {}

  //! \param ftor Invocable object to be wrapped
  template<typename F, UPD_REQUIREMENT(invocable, F)>
  explicit action(F &&ftor) : action{UPD_FWD(ftor), builtin_endianess, builtin_signed_mode} {}

  // @}

  UPD_SFINAE_FAILURE_CTOR(action, UPD_ERROR_NOT_INVOCABLE(ftor))

  using detail::immediate_process<action, void>::operator();

  //! @{
  //! \brief Invoke the managed callback

  //! The parameters are unserialized from the input byte sequence from `src`. After the callback invocation, the result
  //! is serialized into `dest`. \copydoc ImmediateProcess_CRTP
  //!
  //! \param src Input invocable
  //! \param dest Output invocable
  template<typename Src, typename Dest, UPD_REQUIREMENT(input_invocable, Src), UPD_REQUIREMENT(output_invocable, Dest)>
  void operator()(Src &&src, Dest &&dest) const {
    if (m_concept_uptr)
      (*m_concept_uptr)(detail::make_function_reference(src), detail::make_function_reference(dest));
  }

  UPD_SFINAE_FAILURE_MEMBER(operator(), UPD_ERROR_NOT_INPUT(src) " OR " UPD_ERROR_NOT_OUTPUT(dest))

  //! \param input Input byte sequence
  template<typename Input>
  void operator()(Input &&input) const {
    operator()(UPD_FWD(input), [](byte_t) {});
  }

  //! @}

  //! \brief Get the size in bytes of the payload needed to invoke the wrapped object
  //! \return The size of the payload in bytes
  std::size_t input_size() const { return m_concept_uptr->input_size; }

  //! \brief Get the size in bytes of the payload representing the return value of the wrapped invocable object
  //! \return The size of the payload in bytes
  std::size_t output_size() const { return m_concept_uptr->output_size; }

private:
  std::unique_ptr<detail::action_concept> m_concept_uptr;
};

//! \brief Action which does not manage storage for its underlying functor
//!
//! `no_storage_action` instances must be given a free function or a callback with static storage duration, which makes
//! them less permissive than `action` instances. On the other hand, they do not rely on dynamic allocation, which can
//! be useful for embedded software.
class no_storage_action : public detail::immediate_process<no_storage_action, void> {
public:
  using detail::immediate_process<no_storage_action, void>::operator();

  //! @{
  //! \brief Create an action holding the provided callback

  //! \tparam Ftor Free function or callback with static storage duration
  //! \tparam Endianess, Signed_Mode Serialization parameters
  template<typename F, F Ftor, endianess Endianess, signed_mode Signed_Mode>
  explicit no_storage_action(unevaluated<F, Ftor>, endianess_h<Endianess>, signed_mode_h<Signed_Mode>)
      : m_wrapper{detail::static_storage_duration_callback_wrapper<Endianess, Signed_Mode, F, Ftor>},
        m_input_size{detail::parameters_size<F>::value}, m_output_size{detail::return_type_size<F>::value} {}

  //! \tparam Ftor Free function or callback with static storage duration
  template<typename F, F Ftor>
  explicit no_storage_action(unevaluated<F, Ftor>)
      : no_storage_action{unevaluated<F, Ftor>{}, builtin_endianess, builtin_signed_mode} {}

  //! @}

  //! \brief Invoke the managed callback
  //! \copydoc ImmediateProcess_CRTP
  //!
  //! \param src Input invocable
  //! \param dest Output invocable
  template<typename Src, typename Dest, UPD_REQUIREMENT(input_invocable, Src), UPD_REQUIREMENT(output_invocable, Dest)>
  void operator()(Src &&src, Dest &&dest) const {
    m_wrapper(detail::make_function_reference(src), detail::make_function_reference(dest));
  }

  //! \brief Get the size in bytes of the payload needed to invoke the wrapped object
  //! \return The size of the payload in bytes
  std::size_t input_size() const { return m_input_size; }

  //! \brief Get the size in bytes of the payload representing the return value of the wrapped invocable object
  //! \return The size of the payload in bytes
  std::size_t output_size() const { return m_output_size; }

  UPD_SFINAE_FAILURE_MEMBER(operator(), UPD_ERROR_NOT_INPUT(src) " OR " UPD_ERROR_NOT_OUTPUT(dest))

private:
  void (*m_wrapper)(detail::src_t &&, detail::dest_t &&);
  std::size_t m_input_size, m_output_size;
};
} // namespace upd
