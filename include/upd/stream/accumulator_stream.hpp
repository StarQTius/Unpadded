#pragma once

#include <cstddef>
#include <ranges>

#include "../error.hpp"
#include "../upd.hpp"
#include "stream_interface.hpp"
#include <limits.h>

namespace upd {

template<typename BinaryOp>
struct accumulator_stream : stream_interface {
  constexpr explicit accumulator_stream(const BinaryOp *op,
                                        uword_t acc) noexcept(release)
      : op{op}, acc{acc} {}

  auto read(std::size_t, char *) -> lite_error_t override {
    return lite_error::not_implemented;
  }

  auto write(const char *src, std::size_t bitsize) -> lite_error_t override {
    namespace stdr = std::ranges;

    for (auto w : stdr::subrange{src, src + bitsize / CHAR_BIT}) {
      acc = UPD_INVOKE(*op, acc, w);
    }

    return lite_error::none;
  }

  const BinaryOp *op;
  uword_t acc;
};

} // namespace upd
