//! \file
//! \brief 'upd::tuple' template instance deduction from functor

#pragma once

#include <boost/type_traits/remove_cv_ref.hpp>
#include <upd/format.hpp>
#include <upd/tuple.hpp>

#include "signature.hpp"

namespace k2o {
namespace detail {

template<typename, upd::endianess, upd::signed_mode>
struct input_tuple_impl;

template<typename R, typename... Args, upd::endianess Endianess, upd::signed_mode Signed_Mode>
struct input_tuple_impl<R(Args...), Endianess, Signed_Mode> {
  using type = upd::tuple<Endianess, Signed_Mode, boost::remove_cv_ref_t<Args>...>;
};

//! \brief Template instance of 'upd::tuple' suitable for holding the parameters of a functor of type 'F'
//! \tparam Endianess endianess parameter for 'upd::tuple'
//! \tparam Signed_Mode signed representation parameter for 'upd::tuple'
//! \tparam F type of the functor
template<upd::endianess Endianess, upd::signed_mode Signed_Mode, typename F>
using input_tuple = typename input_tuple_impl<signature_t<F>, Endianess, Signed_Mode>::type;

} // namespace detail
} // namespace k2o
