#pragma once

#include "type.hpp"

namespace upd {
namespace system {

enum class Endianess { little, big };
extern const Endianess endianess;

//
inline Endianess get_endianess() {
  unsigned short word = 0xff00;
  return (*reinterpret_cast<byte_t*>(&word) == 0xff ? Endianess::big : Endianess::little );
}

}
} // namespace upd
