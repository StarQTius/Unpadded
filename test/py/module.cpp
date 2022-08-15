#include <upd/keyring.hpp>
#include <upd/python.hpp>

#include "../utility.hpp"

std::uint8_t f1();
void f2(std::uint8_t);
void f3();
std::uint8_t f4(std::uint8_t);

constexpr upd::keyring keyring{upd::flist<f1, f2, f3, f4>, upd::little_endian, upd::two_complement};

PYBIND11_MODULE(module, pymodule) { upd::py::bind(pymodule, keyring); }
