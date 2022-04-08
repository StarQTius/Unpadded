//! \file

#pragma once

#include <upd/format.hpp>

#include "detail/any_function.hpp"
#include "detail/function_reference.hpp"
#include "detail/type_traits/require.hpp"

#include "detail/def.hpp"

namespace k2o {

//! \brief Stores a callback to be called on the response from a device
//! \details This class is non-templated, therefore it is suitable for storage.
class ticket {
  template<typename Index_T, Index_T Index, typename, upd::endianess, upd::signed_mode>
  friend class key;

public:
  //! \brief Call the stored callback on the provided parameters
  //! \param input_invocable Input functor the parameters will be extracted from
  template<typename F, REQUIREMENT(input_invocable, F)>
  void operator()(F &&input_invocable) const {
    m_restorer(m_callback_ptr, detail::make_function_reference<F>(input_invocable));
  }

  //! \copybrief operator()
  //! \param it Start of the range the parameters will be extracted from
  //! \return The error code resulting from the call to 'read_headerless_packet'
  template<typename It, REQUIREMENT(byte_iterator, It)>
  void operator()(It it) const {
    operator()([&]() { return *it++; });
  }

private:
  //! \brief Store a functor convertible to function pointer
  template<typename F, typename Key>
  explicit ticket(F &&ftor, Key) : ticket{+ftor, Key{}} {}

  //! \brief Store a function pointer as callback and its restorer
  template<typename R, typename... Args, typename Key>
  explicit ticket(R (*f_ptr)(Args...), Key)
      : m_callback_ptr{reinterpret_cast<detail::any_function_t *>(f_ptr)},
        m_restorer{detail::restore_and_call<R(Args...), Key>} {}

  detail::any_function_t *m_callback_ptr;
  detail::restorer_t *m_restorer;
};

} // namespace k2o

#include "detail/undef.hpp" // IWYU pragma: keep
