#include "endianess.hpp"

const udp::system::endianess
udp::system::platform_endianess = udp::system::deduce_endianess();
