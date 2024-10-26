#pragma once

#include "upd.hpp"

#include <type_traits>

namespace upd {

template<typename T> requires std::is_reference_v<T>
class ref {
public:
  using type = T;

  constexpr explicit ref(T target) noexcept(release): m_target{UPD_FWD(target)} {}

  [[nodiscard]] constexpr operator T() noexcept(release) {
    return UPD_FWD(m_target);
  }
  
private:
  T m_target;
};

template<typename T>
explicit ref(T &&) -> ref<T &&>;

} // namespace upd
