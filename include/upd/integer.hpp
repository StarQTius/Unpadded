#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "detail/integral_constant.hpp"

namespace upd::detail {

template<typename, typename = void>
struct has_value_member : std::false_type {};

template<typename T>
struct has_value_member<T, std::void_t<decltype(T::value)>> : std::true_type {};

template<typename T>
constexpr auto has_value_member_v = has_value_member<T>::value;

} // namespace upd::detail

namespace upd {

template<typename Bitsize, typename Underlying_T>
class integer_t {
  static_assert(detail::has_value_member_v<Bitsize>, "`Bitsize` must has a `value` member");
  static_assert(std::is_same_v<decltype(Bitsize::value), std::size_t>,
                "`Bitsize::value` must be of type `std::size_t`");
  static_assert(std::is_integral_v<Underlying_T>, "`Underlying_T` must be an integral type");

public:
  constexpr static auto bitsize = Bitsize::value;
  constexpr static auto is_signed = std::is_signed_v<Underlying_T>;

  template<typename T, T Value>
  constexpr integer_t(std::integral_constant<T, Value>) noexcept : m_value{Value} {
    if constexpr (Value > 0) {
      static_assert(Value >> bitsize == 0, "`Value` cannot be represented with `Bitsize` bits");
    } else {
      static_assert(-Value >> bitsize == 0, "`Value` cannot be represented with `Bitsize` bits");
    }
  }

  template<typename T>
  constexpr operator T() const noexcept {
    static_assert(std::is_integral_v<T>, "`T` is not an integer type");
    static_assert(std::numeric_limits<T>::digits >= bitsize, "`T` cannot hold an integer with `Bitsize` bits");

    return m_value;
  }

private:
  Underlying_T m_value;
};

template<std::size_t Bitsize>
using int_t = integer_t<detail::integral_constant_t<Bitsize>, std::intmax_t>;

template<std::size_t Bitsize>
using uint_t = integer_t<detail::integral_constant_t<Bitsize>, std::uintmax_t>;

} // namespace upd
