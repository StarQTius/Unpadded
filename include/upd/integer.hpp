#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <ranges>
#include <type_traits>
#include "detail/ascii_to_integer.hpp"
#include <utility>
#include "detail/always_false.hpp"
#include "is_instance_of.hpp"
#include "token.hpp"
#include "upd.hpp"

namespace upd {

template<std::size_t Bitpos>
constexpr auto nth_bit = 1 << Bitpos;

} // namespace upd

namespace upd {

template<std::size_t, typename Underlying_T>
  requires std::integral<Underlying_T>
class extended_integer;

template<typename T>
  requires std::integral<T>
[[nodiscard]] constexpr auto count_digits(T n) noexcept(release) {
  auto un = static_cast<std::make_unsigned_t<T>>(n);
  auto retval = 0uz;
  for (; un > 0; ++retval, un >>= 1)
    ;
  return retval;
}

template<auto Integer,
         std::size_t Bitsize = count_digits(Integer),
         typename Underlying = std::remove_const_t<decltype(Integer)>>
  requires(count_digits(Integer) <= Bitsize && (Integer > 0 || std::is_signed_v<Underlying>))
constexpr auto xconst = extended_integer<Bitsize, Underlying>{std::in_place, Integer};

namespace detail {

template<typename T, std::size_t Bitsize>
constexpr auto bitmask_v = (T{1} << Bitsize) - 1;

} // namespace detail

template<typename T>
constexpr auto to_extended_integer(T) noexcept;

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
constexpr auto is_extended_integer_v = requires(T x) {
  { extended_integer{x} } -> std::same_as<T>;
};

template<typename Byte, typename XInteger>
  requires(!is_signed_v<Byte> && !is_signed_v<XInteger>)
constexpr auto decompose_into_xuint(XInteger) noexcept(release);

template<typename T>
concept convertible_to_xint = requires(T n) { extended_integer{n}; };

template<std::size_t Bitsize, typename Underlying_T>
  requires std::integral<Underlying_T>
class extended_integer {
  template<std::size_t, typename U>
    requires std::integral<U>
  friend class extended_integer;

  template<typename T>
  friend constexpr auto to_extended_integer(T) noexcept;

  static_assert(std::is_integral_v<decltype(Bitsize)>, "`Bitsize` must be an integer");
  static_assert(Bitsize >= 0, "`Bitsize` must be a positive integer");
  static_assert(Bitsize <= std::numeric_limits<Underlying_T>::digits,
                "`Bitsize` should be lesser or equal to the number of digits in `Underlying_T`");

public:
  using underlying = Underlying_T;

  constexpr static auto bitsize = Bitsize;
  constexpr static auto is_signed = std::is_signed_v<underlying>;

  constexpr extended_integer() noexcept = default;

  template<std::size_t M, typename U>
    requires(M <= bitsize && (std::is_unsigned_v<U> && !is_signed || is_signed))
  constexpr extended_integer(extended_integer<M, U> other) noexcept(release)
      : extended_integer{std::in_place, static_cast<underlying>(other.m_value)} {}

  template<typename T>
    requires(std::integral<T> && std::numeric_limits<T>::digits <= bitsize &&
             (std::is_unsigned_v<T> && !is_signed || is_signed))
  constexpr extended_integer(T n) noexcept(release) : extended_integer{std::in_place, static_cast<underlying>(n)} {}

  constexpr explicit extended_integer(std::in_place_t, Underlying_T n) noexcept(release) : m_value{n} {
    if constexpr (bitsize < std::numeric_limits<Underlying_T>::digits) {
      m_value &= (Underlying_T{1} << bitsize) - 1;
    }
  }

  /*
  [[nodiscard]] constexpr operator bool() const noexcept(release) {
    return static_cast<bool>(m_value);
  }

  template<typename T> requires (
      std::integral<T>
      && std::numeric_limits<underlying>::digits <= bitsize
      && (std::is_unsigned_v<T> && !is_signed || std::is_signed_v<T>))
  constexpr explicit operator T() const noexcept {
    return static_cast<T>(m_value);
  }
  */

  template<std::integral T>
  constexpr explicit operator T() const noexcept(release) {
    auto maybe_value = try_cast<T>(*this);

    UPD_ASSERT(maybe_value);

    return *maybe_value;
  }

  constexpr explicit operator std::byte() const noexcept(release) {
    auto maybe_value = try_cast<std::byte>(*this);

    UPD_ASSERT(maybe_value);

    return *maybe_value;
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
    return xinteger<bitsize, std::make_signed_t<underlying>>{std::in_place,
                                                             static_cast<std::make_signed_t<underlying>>(m_value)};
  }

  [[nodiscard]] constexpr auto abs() const noexcept {
    if constexpr (std::is_unsigned_v<Underlying_T>) {
      return *this;
    } else {
      using abs_underlying = std::make_unsigned_t<Underlying_T>;

      auto value = static_cast<abs_underlying>(m_value);
      auto abs_value = (m_value > 0) ? value : -value;

      return xinteger<bitsize, abs_underlying>{std::in_place, abs_value};
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
    using retval_underlying_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;
    auto retval = xinteger<bitsize - Offset, retval_underlying_type>{};
    retval.m_value = m_value & ((std::uintmax_t{1} << retval.bitsize) - 1);

    return m_value == retval.m_value ? std::optional{retval} : std::nullopt;
  }

  template<std::size_t Offset>
  [[nodiscard]] constexpr auto enlarge(width_t<Offset>) const noexcept {
    using retval_underlying_type = std::conditional_t<is_signed, std::intmax_t, std::uintmax_t>;
    auto retval = xinteger<bitsize + Offset, retval_underlying_type>{};
    retval.m_value = m_value & ((std::uintmax_t{1} << retval.bitsize) - 1);
    return retval;
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
    return xint<bitsize>{std::in_place, -static_cast<std::make_signed_t<underlying>>(m_value)};
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator+(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value + xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{std::in_place, retval};
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator-(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value - xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{std::in_place, retval};
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator/(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value / xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{std::in_place, retval};
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator&(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::min(bitsize, xrhs.bitsize);

    auto retval = m_value & xrhs.m_value;
    return xinteger<retval_bitsize, std::uintmax_t>{std::in_place, retval};
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator^(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value ^ xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{std::in_place, retval};
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator<<(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = bitsize;

    auto retval = m_value << xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{std::in_place, retval};
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator>>(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = bitsize;

    auto retval = m_value >> xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{std::in_place, retval};
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator|(T rhs) const noexcept {
    auto xrhs = to_extended_integer(rhs);
    constexpr auto retval_bitsize = std::max(bitsize, xrhs.bitsize);

    auto retval = m_value | xrhs.m_value;
    using retval_type = decltype(retval);

    return xinteger<retval_bitsize, retval_type>{std::in_place, retval};
  }

  [[nodiscard]] constexpr auto operator~() const noexcept {
    static_assert(!is_signed, "Bitwise operators only work on unsigned values");

    return xinteger<bitsize, underlying>{std::in_place, ~m_value};
  }

  template<typename T>
    requires convertible_to_xint<T>
  constexpr auto operator<<=(T rhs) noexcept -> extended_integer & {
    return (*this = *this << rhs);
  }

  template<typename T>
    requires convertible_to_xint<T>
  constexpr auto operator>>=(T rhs) noexcept -> extended_integer & {
    return (*this = *this >> rhs);
  }

  constexpr auto operator|=(extended_integer rhs) noexcept -> extended_integer & { return (*this = *this | rhs); }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator==(T rhs) const noexcept -> bool {
    auto xrhs = to_extended_integer(rhs);

    return (signbit() == xrhs.signbit() && abs().m_value == xrhs.abs().m_value);
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator<=>(T rhs) const noexcept -> std::strong_ordering {
    auto xrhs = to_extended_integer(rhs);

    if (signbit() != xrhs.signbit()) {
      return signbit() ? std::strong_ordering::less : std::strong_ordering::greater;
    }

    if (!signbit()) {
      return abs().m_value <=> xrhs.abs().m_value;
    } else {
      return xrhs.abs().m_value <=> abs().m_value;
    }
  }

  template<typename T>
    requires convertible_to_xint<T>
  [[nodiscard]] constexpr auto operator<(T rhs) const noexcept -> bool {
    return (*this <=> rhs) < 0;
  }

private:
  Underlying_T m_value;
};

template<typename T>
extended_integer(T) -> extended_integer<std::numeric_limits<T>::digits, T>;

} // namespace upd

namespace upd::literals {

template<char... Cs>
[[nodiscard]] constexpr auto operator""_x() noexcept(release) {
  constexpr auto characters = std::array{Cs...};
  constexpr auto integer = (std::uintmax_t)detail::ascii_to_integer(characters.begin(), characters.end());
  constexpr auto bitsize = [] {
    auto shift_count = std::size_t{0};
    for (auto acc = integer; acc > 0; ++shift_count, acc >>= 1)
      ;
    return shift_count;
  }();

  return xuint<bitsize>{std::in_place, integer};
}

} // namespace upd::literals

namespace upd {

template<typename T>
[[nodiscard]] constexpr auto to_extended_integer(T n) noexcept {
  if constexpr (is_extended_integer_v<T>) {
    return n;
  } else if constexpr (std::is_integral_v<T>) {
    constexpr auto digits = std::numeric_limits<T>::digits;
    return xinteger<digits, T>{std::in_place, n};
  } else if constexpr (std::is_enum_v<T>) {
    using underlying = std::underlying_type_t<T>;
    constexpr auto digits = std::numeric_limits<underlying>::digits;
    return xinteger<digits, underlying>{static_cast<underlying>(n)};
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
  requires(!is_signed_v<Byte> && !is_signed_v<XInteger>)
[[nodiscard]] constexpr auto decompose_into_xuint(XInteger xn) noexcept(release) {
  using namespace literals;
  using underlying = representation_t<Byte>;

  constexpr auto bytewidth = bitsize_v<Byte>;
  constexpr auto byte_count = bitsize_v<XInteger> / bytewidth;
  constexpr auto byte_bitmask = xconst<detail::bitmask_v<underlying, bytewidth>>;

  auto retval = std::array<Byte, byte_count>{};
  for (auto &b : retval) {
    b = xn & byte_bitmask;
    xn >>= bytewidth;
  }

  UPD_ASSERT(xn == 0);

  return retval;
}

template<typename T, std::size_t N>
[[nodiscard]] constexpr auto recompose_into_xuint(const std::array<T, N> &byteseq) noexcept {
  using namespace upd::literals;

  namespace stdr = std::ranges;

  auto retval = (0_x).enlarge(width<bitsize_v<T> * N>);
  auto first = byteseq.rbegin();
  auto last = byteseq.rend();
  auto range = stdr::subrange{first, last};

  for (auto b : range) {
    retval <<= xconst<bitsize_v<T>>;
    if constexpr (std::is_enum_v<T>) {
      retval |= std::to_underlying(b);
    } else {
      retval |= b;
    }
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

template<std::integral T, typename XInt>
  requires(is_instance_of<XInt, extended_integer>())
[[nodiscard]] constexpr auto try_cast(XInt xint) noexcept(release) -> std::optional<T> {
  if (std::in_range<T>(xint.value())) {
    return xint.value();
  } else {
    return std::nullopt;
  }
}

template<typename T, typename XInt>
  requires std::same_as<T, std::byte>
[[nodiscard]] constexpr auto try_cast(XInt xint) noexcept(release) -> std::optional<std::byte> {
  return try_cast<unsigned char>(xint).transform([](auto n) { return std::byte{n}; });
}

} // namespace upd
