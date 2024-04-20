#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace upd {

template<std::size_t Bitsize, typename Underlying_T>
class integer_t {
public:
  template<typename T, T Value>
  constexpr integer_t(std::integral_constant<T, Value>) noexcept : m_value{Value} {
    if constexpr (Value > 0) {
      static_assert(Value >> Bitsize == 0, "`Value` cannot be represented with `Bitsize` bits");
    } else {
      static_assert(-Value >> Bitsize == 0, "`Value` cannot be represented with `Bitsize` bits");
    }
  }

  template<typename T>
  constexpr operator T() const noexcept {
    static_assert(std::is_integral_v<T>, "`T` is not an integer type");
    static_assert(std::numeric_limits<T>::digits >= Bitsize, "`T` cannot hold an integer with `Bitsize` bits");

    return m_value;
  }

private:
  Underlying_T m_value;
};

template<std::size_t Bitsize>
using int_t = integer_t<Bitsize, std::intmax_t>;

template<std::size_t Bitsize>
using uint_t = integer_t<Bitsize, std::uintmax_t>;

} // namespace upd
