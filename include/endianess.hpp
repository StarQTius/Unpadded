#pragma once

#include "type.hpp"

namespace upd {
namespace system {

enum class endianess { LITTLE, BIG };
extern const endianess platform_endianess;

//
inline endianess deduce_endianess() {
  unsigned short word = 0xff00;
  return (*reinterpret_cast<byte_t*>(&word) == 0xff ? endianess::BIG : endianess::LITTLE );
}

}
} // namespace upd
