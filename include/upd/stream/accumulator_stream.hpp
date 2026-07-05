#pragma once

#include <cstddef>
#include <ranges>

#include "../upd.hpp"
#include "stream_interface.hpp"

namespace upd {

template<typename BinaryOp>
struct accumulator_stream : stream_interface {
  constexpr explicit accumulator_stream(const BinaryOp *op, word_t acc) noexcept(release) : op{op}, acc{acc} {}

  auto read(std::size_t, word_t *) -> stream_error_t override { return 1; }

  auto write(const word_t *src, std::size_t size) -> stream_error_t override {
    namespace stdr = std::ranges;

    for (auto w : stdr::subrange{src, src + size}) {
      acc = UPD_INVOKE(*op, acc, w);
    }

    return 0;
  }

  const BinaryOp *op;
  word_t acc;
};

} // namespace upd
