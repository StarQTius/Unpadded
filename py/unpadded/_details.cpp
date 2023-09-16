#include <upd/python.hpp>

using namespace upd;

PYBIND11_MODULE(_details, pymodule) {
  pybind11::class_<action>{pymodule, "_Action"};

  pybind11::enum_<packet_status>{pymodule, "PacketStatus"}
      .value("LOADING_PACKET", packet_status::LOADING_PACKET)
      .value("RESOLVED_PACKET", packet_status::RESOLVED_PACKET)
      .value("DROPPED_PACKET", packet_status::DROPPED_PACKET);
}
