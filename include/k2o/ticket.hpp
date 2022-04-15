//! \file

#pragma once

#include <upd/format.hpp>
#include <upd/type.hpp>

#include "detail/any_function.hpp"
#include "detail/function_reference.hpp"
#include "detail/io/immediate_process.hpp"
#include "detail/static_error.hpp"
#include "detail/type_traits/require.hpp"

#include "detail/def.hpp"

namespace k2o {

//! \brief Stores a callback to be called on the response from a device
//!
//! This class is non-templated, therefore it is suitable for storage.
//!
class ticket : private detail::immediate_process<ticket, void> {
  template<typename Index_T, Index_T Index, typename, upd::endianess, upd::signed_mode>
  friend class key;

public:
  //! \brief Call the stored callback on the provided parameters
  //! \param input Input byte sequence the parameters will be extracted from
  template<typename Input>
  void operator()(Input &&input) {
    operator()(FWD(input), [](upd::byte_t) {});
  }

  K2O_SFINAE_FAILURE_MEMBER(operator(), K2O_ERROR_NOT_INPUT(input))

private:
  //! \brief Store a functor convertible to function pointer
  template<typename F, typename Key>
  explicit ticket(F &&ftor, Key) : ticket{+ftor, Key{}} {}

  //! \brief Store a function pointer as callback and its restorer
  template<typename R, typename... Args, typename Key>
  explicit ticket(R (*f_ptr)(Args...), Key)
      : m_callback_ptr{reinterpret_cast<detail::any_function_t *>(f_ptr)},
        m_restorer{detail::restore_and_call<R(Args...), Key>} {}

  using detail::immediate_process<ticket, void>::operator();

  //! \brief Call the stored callback on the provided parameters
  template<typename Src, typename Dest, REQUIREMENT(input_invocable, Src)>
  void operator()(Src &&src, Dest &&) const {
    m_restorer(m_callback_ptr, detail::make_function_reference<Src>(src));
  }

  detail::any_function_t *m_callback_ptr;
  detail::restorer_t *m_restorer;
};

} // namespace k2o

#include "detail/undef.hpp" // IWYU pragma: keep
