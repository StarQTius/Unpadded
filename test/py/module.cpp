// cppimport

#include <upd/keyring.hpp>
#include <upd/python.hpp>

std::uint8_t f1();
void f2(std::uint8_t);
void f3();
std::uint8_t f4(std::uint8_t);

struct {
  void operator()(std::uint8_t){};
} f5;

auto f6 = [](std::uint8_t) {};

void f7(std::array<int, 16>, std::array<short, 32>);

std::uint16_t g1() { return 0xff; }
std::uint16_t g2(std::uint16_t x) { return 2 * x; }
void g3(std::uint16_t) {}

constexpr upd::keyring keyring{upd::flist<f1, f2, f3, f4, f5, f6, f7>, upd::little_endian, upd::two_complement};
constexpr upd::keyring dispatcher_keyring{upd::flist<g1, g2, g3>, upd::little_endian, upd::two_complement};

PYBIND11_MODULE(module, pymodule) {
  upd::py::unpack_keyring(pymodule, keyring);
  upd::py::declare_dispatcher(pymodule, "Dispatcher", dispatcher_keyring);
}

<% setup_unpadded(cfg) %>
