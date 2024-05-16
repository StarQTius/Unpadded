#pragma once

#include <cstdint>

struct serializer_interface {
  serializer_interface(const serializer_interface &) = delete;
  auto operator=(const serializer_interface &) -> serializer_interface & = delete;

  serializer_interface(serializer_interface &&) = delete;
  auto operator=(serializer_interface &&) -> serializer_interface & = delete;

  virtual void serialize_unsigned(std::uintmax_t value, std::size_t size, std::byte *output) = 0;
  virtual void serialize_signed(std::intmax_t value, std::size_t size, std::byte *output) = 0;
  virtual auto deserialize_unsigned(const std::byte *input, std::size_t size) -> std::uintmax_t = 0;
  virtual auto deserialize_signed(const std::byte *input, std::size_t size) -> std::intmax_t = 0;
  virtual ~serializer_interface() = default;
};
