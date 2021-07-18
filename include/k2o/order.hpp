#pragma once

#include <upd/storage.hpp>

#include <k2o/detail/function_reference.hpp>
#include <k2o/detail/sfinae.hpp>
#include <k2o/detail/signature.hpp>
#include <k2o/detail/unique_ptr.hpp>

#include "status.hpp"
#include "type.hpp"

//! \file

namespace k2o {
namespace detail {

//!
template<typename, upd::endianess, upd::signed_mode>
struct input_tuple_impl;
template<typename R, typename... Args, upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct input_tuple_impl<R(Args...), Endianess, Signed_Mode> {
  using type = upd::tuple<Endianess, Signed_Mode, Args...>;
};

//!
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename F>
using input_tuple = typename input_tuple_impl<signature_t<F>, Endianess, Signed_Mode>::type;

//!
using src_t = abstract_function<byte_t()>;

//!
using dest_t = abstract_function<void(byte_t)>;

//!
class order_concept {
public:
  virtual ~order_concept() = default;
  virtual status operator()(src_t &&) = 0;
  virtual status operator()(src_t &&, dest_t &&) = 0;
};

template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename T, sfinae::require_not_tuple<T> = 0>
status insert(dest_t& insert_byte, const T &value) {
  using namespace upd;

  auto output = make_tuple<Endianess, Signed_Mode>(value);
  for (byte_t byte : output) insert_byte(byte);

  return status::OK;
}
template<upd::endianess, upd::signed_mode, typename T, sfinae::require_is_tuple<T> = 0>
status insert(dest_t &insert_byte, const T &tuple) {
  using namespace upd;

  for (byte_t byte : tuple) insert_byte(byte);

  return status::OK;
}

//!
template<typename Tuple, typename F>
status call(src_t &fetch_byte, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args) byte = fetch_byte();
  input_args.invoke(FWD(ftor));

  return status::OK;
}

//!
template<typename Tuple, typename F, sfinae::require_is_void<detail::return_t<F>> = 0>
status call(src_t &fetch_byte, dest_t &, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args) byte = fetch_byte();
  input_args.invoke(FWD(ftor));

  return status::OK;
}

//!
template<typename Tuple, typename F, sfinae::require_not_void<detail::return_t<F>> = 0>
status call(src_t &fetch_byte, dest_t &insert_byte, F &&ftor) {
  Tuple input_args;
  for (auto &byte : input_args) byte = fetch_byte();

  return insert(insert_byte, input_args.invoke(FWD(ftor)));
}

//!
template<typename F, upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct order_model_impl {
  //!
  using tuple_t = input_tuple<Endianess, Signed_Mode, F>;

  //!
  explicit order_model_impl(const F &ftor) : ftor{ftor} {}

  //!
  explicit order_model_impl(F &&ftor) : ftor{static_cast<F &&>(ftor)} {}

  //!
  F ftor;
};

//!
template<typename F, upd::endianess Endianess, upd::signed_mode Signed_Mode>
class order_model : public order_concept {
  using impl_t = order_model_impl<F, Endianess, Signed_Mode>;
  using tuple_t = typename impl_t::tuple_t;

public:
  //!
  explicit order_model(F &&ftor) : m_impl{FWD(ftor)} {}

  //!
  status operator()(src_t &&fetch_byte) final {
    return detail::call<tuple_t>(fetch_byte, FWD(m_impl.ftor));
  }

  //!
  status operator()(src_t &&fetch_byte, dest_t &&insert_byte) final {
    return detail::call<tuple_t>(fetch_byte, insert_byte, FWD(m_impl.ftor));
  }

private:
  impl_t m_impl;
};

} // namespace detail

//!
class order {
public:
  //!
  template<upd::endianess Endianess = upd::endianess::BUILTIN,
           upd::signed_mode Signed_Mode = upd::signed_mode::BUILTIN,
           typename F>
  explicit order(F &&ftor) : m_concept_uptr{new detail::order_model<F, Endianess, Signed_Mode>{FWD(ftor)}} {}

  //!
  template<typename Src_F>
  status operator()(Src_F &&fetch_byte) {
    return (*m_concept_uptr)(detail::make_function_reference(fetch_byte));
  }

  //!
  template<typename Src_F, typename Dest_F>
  status operator()(Src_F &&fetch_byte, Dest_F &&insert_byte) {
    return (*m_concept_uptr)(
      detail::make_function_reference(fetch_byte),
      detail::make_function_reference(insert_byte));
  }

private:
  detail::unique_ptr<detail::order_concept> m_concept_uptr;
};

} // namespace k2o
