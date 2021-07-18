//! \file

#pragma once

#include <boost/type_traits.hpp>

#include "signature.hpp"

namespace k2o {
namespace detail {

template<typename>
struct abstract_function;

//!
template<typename R, typename... Args>
struct abstract_function<R(Args...)> {
  virtual ~abstract_function() = default;
  virtual R operator()(boost::remove_reference_t<Args> &...) = 0;
};

//!
template<typename...>
class function_reference_base;

//!
template<typename F, typename R, typename... Args>
class function_reference_base<F, R(Args...)> : public abstract_function<R(Args...)> {
public:
  //!
  function_reference_base(F &ftor) : m_ftor{ftor} {}

  //!
  R operator()(Args &&... args) final { return m_ftor(static_cast<Args>(args)...); }

private:
  F &m_ftor;
};

//!
template<typename F>
class function_reference : public function_reference_base<F, signature_t<F>> {
public:
  using function_reference_base<F, signature_t<F>>::function_reference_base;
};

//!
template<typename F>
function_reference<F> make_function_reference(F &ftor) {
  return function_reference<F>(ftor);
}

} // namespace detail
} // namespace k2o
