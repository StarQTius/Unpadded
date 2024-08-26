#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <algorithm>
#include <optional>
#include <cmath>

#include "detail/integral_constant.hpp"
#include "description.hpp"
#include "literals.hpp"
#include "detail/is_instance_of.hpp"
#include "detail/always_false.hpp"

namespace upd::detail {

template<typename, typename = void>
struct has_value_member : std::false_type {};

template<typename T>
struct has_value_member<T, std::void_t<decltype(T::value)>> : std::true_type {};

template<typename T>
constexpr auto has_value_member_v = has_value_member<T>::value;

template<typename T, std::size_t Bitsize>
constexpr auto bitmask_v = ((T) 1 << Bitsize) - 1;

} // namespace upd::detail

namespace upd {

template<std::size_t Bitpos>
constexpr auto nth_bit = 1 << Bitpos;

} // namespace upd

namespace upd {

template<typename, typename>
class extended_integer;

template<typename T>
constexpr auto to_extended_integer(T) noexcept;

template<std::size_t Bitsize, typename Underlying>
using xinteger = extended_integer<detail::integral_constant_t<Bitsize>, Underlying>;

template<typename Underlying>
using xinteger_fit = xinteger<std::numeric_limits<Underlying>::digits, Underlying>;

template<std::size_t Bitsize>
using xint = extended_integer<detail::integral_constant_t<Bitsize>, std::intmax_t>;

template<std::size_t Bitsize>
using xuint = extended_integer<detail::integral_constant_t<Bitsize>, std::uintmax_t>;

template<typename T>
constexpr auto bitsize_v = decltype(to_extended_integer(std::declval<T>()))::bitsize;

template<typename T>
constexpr auto is_signed_v = decltype(to_extended_integer(std::declval<T>()))::is_signed;

template<typename T>
constexpr auto is_extended_integer_v = detail::is_instance_of_v<T, extended_integer>;

template<typename Bitsize, typename Underlying_T>
class extended_integer {
  template<typename, typename>
  friend class extended_integer;

  template<typename T>
  friend constexpr auto to_extended_integer(T) noexcept;

  static_assert(detail::has_value_member_v<Bitsize>, "`Bitsize` must has a `value` member");
  static_assert(std::is_integral_v<decltype(Bitsize::value)>, "`Bitsize::value` must be an integer");
  static_assert(Bitsize::value >= 0,
                "`Bitsize::value` must be a positive integer");
  static_assert(std::is_integral_v<Underlying_T>, "`Underlying_T` must be an integral type");
  static_assert(Bitsize::value <= std::numeric_limits<Underlying_T>::digits, "`Bitsize::value` should be lesser or equal to the number of digits in `Underlying_T`");

public:
  using underlying = Underlying_T;

  constexpr static auto bitsize = Bitsize::value;
  constexpr static auto is_signed = std::is_signed_v<underlying>;

  constexpr extended_integer() noexcept = default;
  
  template<typename T>
  constexpr extended_integer(T n) noexcept {
    static_assert(std::is_integral_v<T>, "`n` must be an integer");
    
    using pow2_type = std::conditional_t<std::is_signed_v<T>, std::intmax_t, std::uintmax_t>;
    
    auto pow2 = (pow2_type) 1 << bitsize;
    m_value = static_cast<Underlying_T>(n % pow2);
  }

  template<typename T>
  constexpr operator T() const noexcept {
    static_assert(std::is_integral_v<T>, "`T` is not an integer type");
    static_assert(std::numeric_limits<T>::digits >= bitsize, "`T` cannot hold an integer with `Bitsize` bits");

    return m_value;
  }

  [[nodiscard]] constexpr auto value() const noexcept -> underlying {
    return m_value;
  }

  template<std::size_t Bytewidth>
  [[nodiscard]] constexpr auto decompose(std::integral_constant<std::size_t, Bytewidth>) const noexcept {
    using byte = xinteger<Bytewidth, Underlying_T>;

    return decompose<byte>();
  }

  template<typename Byte>
  [[nodiscard]] constexpr auto decompose() const {
    using namespace literals;

    constexpr auto bytewidth = bitsize_v<Byte>;

    static_assert(!is_signed, "`decompose()` only works on unsigned values");
    static_assert(!is_signed_v<Byte>, "`Byte` must be unsigned");
    static_assert(bitsize % bytewidth == 0, "The value must be decomposable into a sequence of `Byte` values without any padding");

    constexpr auto byte_count = bitsize / bytewidth;
    constexpr auto byte_bitmask = detail::bitmask_v<Underlying_T, bytewidth>;

    auto retval = std::array<Byte, byte_count>{};
    auto value = m_value;
    auto first = retval.rbegin();
    auto last = retval.rend();
    auto range = detail::range{first, last};

    for (auto &b : range) {
      b = value & byte_bitmask;
      value >>= bitsize_v<Byte>;
    }

    UPD_ASSERT(value == 0);

    return retval;
  }

  [[nodiscard]] constexpr auto abs() const noexcept {
    if constexpr (std::is_unsigned_v<Underlying_T>) {
      return *this;
    } else {
      using abs_underlying = std::make_unsigned_t<Underlying_T>;

      auto value = static_cast<abs_underlying>(m_value);
      auto abs_value = (m_value > 0) ? value : -value;

      return xinteger<bitsize - 1, abs_underlying>{abs_value};
    }
  }

  [[nodiscard]] constexpr auto signbit() const noexcept -> bool {
    return std::signbit(m_value);
  }

  template<std::size_t NewBitsize>
  [[nodiscard]] constexpr auto resize(width_t<NewBitsize>) const noexcept {
    if (NewBitsize < bitsize) {
      return shrink(width<bitsize - NewBitsize>);
    } else {
      auto retval = enlarge(width<NewBitsize - bitsize>);
      return std::optional{retval};
    }
  }

  template<std::size_t Offset>
  [[nodiscard]] constexpr auto shrink(width_t<Offset>) const noexcept {
    auto retval = xinteger<bitsize - Offset, underlying>{m_value};

    return m_value == retval.m_value ? std::optional{retval} : std::nullopt;
  }

  template<std::size_t Offset>
  [[nodiscard]] constexpr auto enlarge(width_t<Offset>) const noexcept {
    return xinteger<bitsize + Offset, underlying>{m_value};
  }

  template<typename T>
  [[nodiscard]] constexpr auto operator+(T rhs) const noexcept {
    constexpr auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value + xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{retval};
  }

  [[nodiscard]] constexpr auto operator~() const noexcept {
    static_assert(!is_signed, "Bitwise operators only work on unsigned values");
  
    return xinteger<bitsize, underlying>{~m_value};
  }

  constexpr auto operator<<=(std::size_t offset) noexcept -> extended_integer& {
    static_assert(!is_signed, "Bitwise operators only work on unsigned values");
  
    m_value <<= offset;

    return *this;
  }

  template<typename T>
  constexpr auto operator|=(T rhs) noexcept -> extended_integer& {
    auto xrhs = to_extended_integer(rhs);
    
    static_assert(!is_signed && !xrhs.is_signed, "Bitwise operators only work on unsigned values");
    static_assert(xrhs.bitsize <= bitsize, "Such an operation would cause a narrowing conversion");
  
    m_value |= xrhs.m_value;

    return *this;
  }

  template<typename T>
  [[nodiscard]] constexpr auto operator==(T rhs) const noexcept -> bool {
    auto xrhs = to_extended_integer(rhs);

    return (signbit() == xrhs.signbit() && abs().m_value == xrhs.abs().m_value);
  }

private:
  Underlying_T m_value;
};

template<typename T>
[[nodiscard]] constexpr auto to_extended_integer(T n) noexcept {
  if constexpr (detail::is_instance_of_v<T, extended_integer>) {
    return n;
  } else if constexpr (std::is_integral_v<T>) {
    constexpr auto digits = std::numeric_limits<T>::digits;
    return xinteger<digits, T>{n};
  } else {
    static_assert(false, "`n` must be an `extended_integer` instance or integral value");
  }
}

template<typename T, std::size_t N>
[[nodiscard]] constexpr auto recompose_into_xuint(const std::array<T, N> &byteseq) noexcept {
  using namespace upd::literals;
  
  auto retval = xuint<bitsize_v<T> * N>{0};
  auto first = byteseq.rbegin();
  auto last = byteseq.rend();
  auto range = detail::range{first, last};

  for (auto b : range) {
    retval <<= bitsize_v<T>;
    retval |= b;
  }

  return retval;
}

} // namespace upd
