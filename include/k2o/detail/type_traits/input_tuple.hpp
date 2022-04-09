//! \file

#pragma once

#include <upd/format.hpp>
#include <upd/tuple.hpp>

#include "remove_cv_ref.hpp"
#include "signature.hpp"

namespace k2o {
namespace detail {

template<typename, upd::endianess, upd::signed_mode>
struct input_tuple_impl;

template<typename R, typename... Args, upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct input_tuple_impl<R(Args...), Endianess, Signed_Mode> {
  using type = upd::tuple<Endianess, Signed_Mode, remove_cv_ref_t<Args>...>;
};

//! \brief Template instance of `upd::tuple` suitable for holding the parameters of an invocable of type `F`
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename F>
using input_tuple = typename input_tuple_impl<signature_t<F>, Endianess, Signed_Mode>::type;

} // namespace detail
} // namespace k2o
