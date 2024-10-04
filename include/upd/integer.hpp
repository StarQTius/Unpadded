#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

#include "detail/always_false.hpp"
#include "detail/has_value_member.hpp"
#include "detail/integral_constant.hpp"
#include "detail/is_instance_of.hpp"
#include "literals.hpp"
#include "token.hpp"
#include "upd.hpp"

namespace upd::detail {

template<typename T, std::size_t Bitsize>
constexpr auto bitmask_v = ((T)1 << Bitsize) - 1;

} // namespace upd::detail

namespace upd {

template<std::size_t Bitpos>
constexpr auto nth_bit = 1 << Bitpos;

} // namespace upd

namespace upd {

template<std::size_t, typename>
class extended_integer;

template<typename T>
constexpr auto to_extended_integer(T) noexcept;

template<typename, typename XInteger>
constexpr auto decompose_into_xuint(XInteger) noexcept(release);

template<typename Scalar>
[[nodiscard]] constexpr auto reduce_scalar(Scalar, std::size_t) noexcept;

template<typename Integer>
[[nodiscard]] constexpr auto reduce_integer(Integer n, std::size_t bitsize) noexcept;

template<typename Enum>
[[nodiscard]] constexpr auto reduce_enum(Enum e, std::size_t bitsize) noexcept;

template<std::size_t Bitsize, typename Underlying>
using xinteger = extended_integer<Bitsize, Underlying>;

template<typename Underlying>
using xinteger_fit = xinteger<std::numeric_limits<Underlying>::digits, Underlying>;

template<std::size_t Bitsize>
using xint = extended_integer<Bitsize, std::intmax_t>;

template<std::size_t Bitsize>
using xuint = extended_integer<Bitsize, std::uintmax_t>;

template<typename T>
using representation_t = typename decltype(to_extended_integer(std::declval<T>()))::underlying;

template<typename T>
constexpr auto bitsize_v = decltype(to_extended_integer(std::declval<T>()))::bitsize;

template<typename T>
constexpr auto is_signed_v = decltype(to_extended_integer(std::declval<T>()))::is_signed;

template<typename T>
constexpr auto is_extended_integer_v = requires(T x) { {extended_integer{x}} -> std::same_as<T>; };

template<std::size_t Bitsize, typename Underlying_T>
class extended_integer {
  template<std::size_t, typename>
  friend class extended_integer;

  template<typename T>
  friend constexpr auto to_extended_integer(T) noexcept;

  static_assert(std::is_integral_v<decltype(Bitsize)>, "`Bitsize` must be an integer");
  static_assert(Bitsize >= 0, "`Bitsize` must be a positive integer");
  static_assert(std::is_integral_v<Underlying_T>, "`Underlying_T` must be an integral type");
  static_assert(Bitsize <= std::numeric_limits<Underlying_T>::digits,
                "`Bitsize` should be lesser or equal to the number of digits in `Underlying_T`");

public:
  using underlying = Underlying_T;

  constexpr static auto bitsize = Bitsize;
  constexpr static auto is_signed = std::is_signed_v<underlying>;

  constexpr extended_integer() noexcept = default;

  template<typename T>
  constexpr extended_integer(T n) noexcept(release) {
    if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) {
      m_value = (underlying) reduce_scalar(n, bitsize);
    } else if constexpr (is_extended_integer_v<T>) {
      m_value = (underlying) reduce_scalar(n.m_value, bitsize);
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`n` must be an integer, an enumerator, or an extended integer");
    }
  }

  [[nodiscard]] constexpr operator bool() const noexcept(release) {
    return static_cast<bool>(m_value);
  }

  template<typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_enum_v<T>>>
  constexpr operator T() const noexcept {
    if constexpr (std::is_integral_v<T>) {
      static_assert(std::numeric_limits<T>::digits >= bitsize, "`T` cannot hold an integer with `Bitsize` bits");
      return static_cast<T>(m_value);
    } else {
      using underlying = std::underlying_type_t<T>;
      static_assert(std::numeric_limits<underlying>::digits >= bitsize,
                    "`T` cannot hold an integer with `Bitsize` bits");
      return static_cast<T>(m_value);
    }
  }

  [[nodiscard]] constexpr auto value() const noexcept -> underlying { return m_value; }

  template<std::size_t Bytewidth>
  [[nodiscard]] constexpr auto decompose(std::integral_constant<std::size_t, Bytewidth>) const noexcept {
    using byte = xinteger<Bytewidth, underlying>;

    return decompose_into_xuint<byte>(*this);
  }

  template<std::size_t ByteCount>
  [[nodiscard]] constexpr auto decompose(width_t<ByteCount>) const noexcept(release) {
    using byte = xinteger<bitsize / ByteCount, underlying>;

    return decompose_into_xuint<byte>(*this);
  }

  [[nodiscard]] constexpr auto as_signed() const noexcept(release) {
    return xinteger<bitsize, std::make_signed_t<underlying>>{m_value};
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

  [[nodiscard]] constexpr auto signbit() const noexcept -> bool { return std::signbit(m_value); }

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

  template<typename Serializer, typename OutputIt>
  constexpr void serialize(Serializer &ser, OutputIt dest) const noexcept {
    if constexpr (is_signed) {
      ser.serialize_signed(*this, dest);
    } else {
      ser.serialize_unsigned(*this, dest);
    }
  }

  [[nodiscard]] constexpr auto operator-() const noexcept(release) {
    return xint<bitsize>{-m_value};
  }

  template<typename T>
  [[nodiscard]] constexpr auto operator+(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value + xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{retval};
  }

  template<typename T>
  [[nodiscard]] constexpr auto operator&(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value & xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{retval};
  }

  template<typename T>
  [[nodiscard]] constexpr auto operator^(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value ^ xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{retval};
  }

  template<typename T>
  [[nodiscard]] constexpr auto operator<<(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value << xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{retval};
  }

  template<typename T>
  [[nodiscard]] constexpr auto operator>>(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value >> xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{retval};
  }

  [[nodiscard]] constexpr auto operator~() const noexcept {
    static_assert(!is_signed, "Bitwise operators only work on unsigned values");

    return xinteger<bitsize, underlying>{~m_value};
  }

  constexpr auto operator<<=(std::size_t offset) noexcept -> extended_integer & {
    static_assert(!is_signed, "Bitwise operators only work on unsigned values");

    m_value <<= offset;

    return *this;
  }

  template<typename T>
  constexpr auto operator|=(T rhs) noexcept -> extended_integer & {
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
  if constexpr (is_extended_integer_v<T>) {
    return n;
  } else if constexpr (std::is_integral_v<T>) {
    constexpr auto digits = std::numeric_limits<T>::digits;
    return xinteger<digits, T>{n};
  } else if constexpr (std::is_enum_v<T>) {
    using underlying = std::underlying_type_t<T>;
    constexpr auto digits = std::numeric_limits<underlying>::digits;
    return xinteger<digits, underlying>{n};
  } else {
    static_assert(UPD_ALWAYS_FALSE, "`n` must be an `extended_integer` instance or integral value");
  }
}

template<typename XInteger, typename InputIt, typename Serializer>
[[nodiscard]] constexpr auto deserialize_into_xinteger(InputIt src, Serializer &ser) -> XInteger {
  if constexpr (XInteger::is_signed) {
    return ser.deserialize_signed(src, width<XInteger::bitsize>);
  } else {
    return ser.deserialize_unsigned(src, width<XInteger::bitsize>);
  }
}

template<typename Byte, typename XInteger>
[[nodiscard]] constexpr auto decompose_into_xuint(XInteger xn) noexcept(release) {
  using namespace literals;

  static_assert(!is_signed_v<XInteger>, "`decompose()` only works on unsigned values");
  static_assert(!is_signed_v<Byte>, "Can only decompose `xn` into sequence of unsigned values");

  using underlying = representation_t<Byte>;

  constexpr auto bytewidth = bitsize_v<Byte>;
  constexpr auto byte_count = bitsize_v<XInteger> / bytewidth;
  constexpr auto byte_bitmask = detail::bitmask_v<underlying, bytewidth>;

  auto retval = std::array<Byte, byte_count>{};
  auto value = xn.value();

  for (auto &b : retval) {
    b = value & byte_bitmask;
    value >>= bytewidth;
  }

  UPD_ASSERT(value == 0);

  return retval;
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

template<typename Scalar>
[[nodiscard]] constexpr auto reduce_scalar(Scalar s, std::size_t bitsize) noexcept {
  static_assert(std::is_scalar_v<Scalar>, "`s` must be a scalar type");

  if constexpr (std::is_integral_v<Scalar>) {
    return reduce_integer(s, bitsize);
  } else if constexpr (std::is_enum_v<Scalar>) {
    return reduce_enum(s, bitsize);
  } else {
    static_assert(UPD_ALWAYS_FALSE, "`s` has an unsupported type");
  }
}

template<typename Integer>
[[nodiscard]] constexpr auto reduce_integer(Integer n, std::size_t bitsize) noexcept {
  static_assert(std::is_integral_v<Integer>, "`n` must be an integer");

  using pow2_type = std::conditional_t<std::is_signed_v<Integer>, std::intmax_t, std::uintmax_t>;

  auto pow2 = (pow2_type)1 << bitsize;

  return n % pow2;
}

template<typename Enum>
[[nodiscard]] constexpr auto reduce_enum(Enum e, std::size_t bitsize) noexcept {
  static_assert(std::is_enum_v<Enum>, "`e` must be an enumerator");

  using underlying = std::underlying_type_t<Enum>;

  auto value = static_cast<underlying>(e);

  return reduce_integer(value, bitsize);
}

} // namespace upd

namespace upd::literals {

template<char... Cs>
[[nodiscard]] constexpr auto operator ""_xui() noexcept(release) {
  constexpr auto characters = std::array {Cs...};
  constexpr auto integer = (std::uintmax_t) detail::ascii_to_integer(characters.begin(), characters.end());
  constexpr auto bitsize = [] {
    auto shift_count = std::size_t{0};
    for (auto acc = integer; acc > 0; ++shift_count, acc >>= 1);
    return shift_count;
  }();

  return xuint<bitsize>{integer};
} 

} // namespace upd::literals
