#pragma once

#include <algorithm> // IWYU pragma: keep
#include <array>
#include <cstddef>

#include "detail/always_false.hpp"
#include "detail/is_bounded_array.hpp" // IWYU pragma: keep
#include "upd.hpp"

namespace upd {

template<typename Consumer_T, typename Serializer_T>
class basic_obytestream {
public:
  basic_obytestream(Consumer_T consumer, Serializer_T serializer) noexcept
      : m_consumer{UPD_FWD(consumer)}, m_serializer{UPD_FWD(serializer)} {}

  template<typename... Args>
  void encode(const Args &...args) {
    if constexpr (sizeof...(args) > 1) {
      (encode(UPD_FWD(args)), ...);
    } else {
      encode_one(UPD_FWD(args)...);
    }
  }

private:
  template<typename T>
  void encode_one(const T &x) {
    if constexpr (std::is_signed_v<T>) {
      encode_signed(UPD_FWD(x));
    } else if constexpr (std::is_unsigned_v<T>) {
      encode_unsigned(UPD_FWD(x));
    } else if constexpr (detail::is_bounded_array_v<T>) {
      encode_bounded_array(UPD_FWD(x));
    } else {
      static_assert(UPD_ALWAYS_FALSE, "`T` cannot be serialized");
    }
  }

  template<typename T>
  void encode_signed(const T &value) {
    auto buf = std::array<std::byte, sizeof value>{};

    m_serializer.serialize_signed(UPD_FWD(value), buf.size(), buf.data());
    m_consumer = std::copy(buf.begin(), buf.end(), m_consumer);
  };

  template<typename T>
  void encode_unsigned(const T &value) {
    auto buf = std::array<std::byte, sizeof value>{};

    m_serializer.serialize_unsigned(UPD_FWD(value), buf.size(), buf.data());
    m_consumer = std::copy(buf.begin(), buf.end(), m_consumer);
  };

  template<typename T>
  void encode_bounded_array(const T &array) {
    for (const auto &value : array) {
      encode_one(value);
    }
  };

  Consumer_T m_consumer;
  Serializer_T m_serializer;
};

} // namespace upd
