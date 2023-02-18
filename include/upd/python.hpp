#pragma once

#include <cstdlib>
#include <cxxabi.h>
#include <functional>
#include <regex>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>

#include <pybind11/cast.h>
#include <pybind11/functional.h> // IWYU pragma: keep
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h> // IWYU pragma: keep

#include "action.hpp"
#include "buffered_dispatcher.hpp"
#include "detail/type_traits/require.hpp"
#include "detail/type_traits/typelist.hpp"
#include "format.hpp"
#include "key.hpp"
#include "policy.hpp"
#include "type.hpp"

// IWYU pragma: no_include <pybind11/detail/descr.h>

namespace upd {
namespace py {
namespace detail {

//! \brief Prefix appended to the key names in order to get their name
constexpr char pykey_prefix[] = "__";

//! \brief Demangle the name of a symbol wrapped in an \ref<unevaluated> unevaluated template instance
template<typename T>
std::string demangle(const T &x) {
  int status;
  auto *cstr_name = abi::__cxa_demangle(typeid(x).name(), NULL, NULL, &status);

  if (!cstr_name)
    throw std::runtime_error{(std::string) "Name type `" + typeid(x).name() + "` couldn't be demangled"};

  std::string name{cstr_name};
  free(cstr_name);

  // Grab [CALLBACK REFERENCE] in `upd::unevaluated<[CALLBACK TYPE], [CALLBACK REFERENCE]>` and remove the potential
  // return type, parameters types and the `&` symbol
  std::regex callback_name_re{"upd::unevaluated<.*,\\s\\&\\(?(\\w+)(\\(.*\\))?\\)?>$"};
  std::smatch match;
  if (std::regex_match(name, match, callback_name_re) && match.size() == 3) {
    return match[1];
  } else {
    throw std::invalid_argument{(std::string) "No callback name could have been extracted from `" + name + "`"};
  }
}

//! \brief Get the demangled name of a symbol wrapped in an \ref<unevaluated> unevaluated template instance as C-style
//! string
//!
//! The returned string is kept valid until the end of the execution of the program as if it had static storage
//! duration.
template<typename T>
const char *demangled_fname(const T &x) {
  static std::string static_name{pykey_prefix + demangle(x)};

  return static_name.c_str();
}

//! \brief Expose a \ref<key> key template instance to Python
template<typename Keyring, typename Index_T, Index_T I, typename R, typename... Args, endianess E, signed_mode S>
void define_pykey(pybind11::module &pymodule, const char *name, Keyring, key<Index_T, I, R(Args...), E, S> k) {
  using key_t = decltype(k);

  pybind11::class_<key_t>{pymodule, name}
      .def("encode",
           [](key_t k, const Args &...args) {
             byte_t buf[key_t::payload_length];

             k(args...).write_to(buf);
             return pybind11::bytes{reinterpret_cast<char *>(buf), sizeof buf};
           })
      .def("decode",
           [](key_t k, pybind11::object &pybytes) {
             auto it = pybytes.begin();
             return k.read_from([&]() {
               ++it;
               return it->cast<byte_t>();
             });
           })
      .def("index", [](key_t) { return I; })
      .def("__make_action", [](key_t, pybind11::function pyfunction) {
        return action{[pyfunction](const Args &...args) {
          if constexpr (std::is_void_v<R>) {
            pyfunction(args...);
          } else {
            return pyfunction(args...).template cast<R>();
          }
        }};
      });

  pymodule.attr(name + 2) = k;
}

//! \brief Expose the \ref<key> key template instances of a \ref<keyring> keyring template instance to Python
template<typename Keyring, std::size_t... Is>
void define_pykeys(pybind11::module &pymodule, Keyring keyring, std::index_sequence<Is...>) {
  using flist_t = typename Keyring::flist_t;

  using discard = int[];
  discard{0,
          (define_pykey(pymodule,
                        demangled_fname(upd::detail::at<flist_t, Is>{}),
                        keyring,
                        keyring.get(upd::detail::at<flist_t, Is>{})),
           0)...};
}

} // namespace detail

//! \brief Expose every \ref<key> key template instances from a keyring to Python
//!
//! For every key in the provided keyring, a Python class with the same name as the callback held by the key will be
//! defined with the following methods :
//!
//!   - `encode(self, *args) -> bytes` : serialize an argument list into a sequence of bytes
//!   - `decode(self, payload: bytes)` : deserialize a sequence of bytes and return the corresponding value
//!   - `index(self) -> int` : get the index associated with the key
//!
//! \tparam Keyring Keyring holding the keys to expose
//! \param pymodule Python module inside of which the classes will be defined
template<typename Keyring, UPD_REQUIREMENT(is_keyring, Keyring)>
void unpack_keyring(pybind11::module &pymodule, Keyring keyring) {
  detail::define_pykeys(pymodule, keyring, std::make_index_sequence<Keyring::size>{});
}

//! \brief Expose a \ref<double_buffered_dispatcher> double_buffered_dispatcher template instance made from the given
//! keyring
//!
//! The class will have the following methods :
//!   - `read_from(self, getter: Callable[[], int]) -> unpadded.PacketStatus`: read from a byte getter until a packet is
//!   dropped on resolved
//!   - `write_to(self, putter: Callable[[int], None]) -> None`: write to a byte putter until the internal output buffer
//!   is exhausted
//!   - `put(self, x: int) -> None`: put a single byte into the input internal buffer and return the status of the
//!   current packet
//!   - `get(self) -> int`: get a single byte from the ouput interna buffer
//!   - `is_loaded(self) -> bool`: check whether a packet is loaded in the output buffer
//!   - `replace(self, key, f: Callable) -> None`: replace the behavior of the action designated by `key` by `f` (Python
//!   callbacks are supported)
//!
//! \note The provided keyring is unpacked.
//!
//! \tparam Keyring Keyring holding the keys to expose
//! \param name Name for the Python class bound to the dispatcher
//! \param pymodule Python module inside of which the classes will be defined
template<typename Keyring, UPD_REQUIREMENT(is_keyring, Keyring)>
void declare_dispatcher(pybind11::module &pymodule, const char *name, Keyring keyring) {
  using namespace pybind11::literals;
  using dispatcher_t = decltype(double_buffered_dispatcher{Keyring{}, policy::any_callback});

  pybind11::class_<dispatcher_t>{pymodule, name}
      .def(pybind11::init<>())
      .def("read_from", [](dispatcher_t &self, const std::function<byte_t()> &src) { return self.read_from(src); })
      .def("write_to", [](dispatcher_t &self, const std::function<void(byte_t)> &dest) { self.write_to(dest); })
      .def("put", &dispatcher_t::put)
      .def("get", &dispatcher_t::get)
      .def("is_loaded", &dispatcher_t::is_loaded)
      .def("replace", [](dispatcher_t &self, pybind11::object pykey, pybind11::function pyfunction) {
        self[pykey.attr("index")().cast<std::size_t>()] = pykey.attr("__make_action")(pyfunction).cast<action>();
      });

  unpack_keyring(pymodule, keyring);
}

} // namespace py
} // namespace upd
