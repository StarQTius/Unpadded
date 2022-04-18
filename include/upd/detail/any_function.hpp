//! \file

#pragma once

#include <upd/type.hpp>

#include "function_reference.hpp"

namespace upd {
namespace detail {

//! \brief Arbitrary function type
//!
//! When forming a pointer to this type, the result may not necessarly be of the same type as 'void *' on Von Neumann
//! architectures. It should also be large enough to store a data pointer. Alignment might be a problem though in some
//! case.
using any_function_t = void();

//! \brief Common type of 'restore_and_call' template instances
using restorer_t = void(any_function_t *, abstract_function<byte_t()> &&);

//! \brief Restore the original type of a callback and call it on the arguments got from the provided input functor
template<typename F, typename Key>
void restore_and_call(any_function_t *callback_ptr, abstract_function<byte_t()> &&input_invocable) {
  auto &callback = *reinterpret_cast<F *>(callback_ptr);
  auto value = Key{} << input_invocable;
  callback(value);
}

} // namespace detail
} // namespace upd
