#pragma once

#include <cstdlib>
#include <cxxabi.h>
#include <functional>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include <pybind11/cast.h>
#include <pybind11/eval.h>
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
#include "upd.hpp"

// IWYU pragma: no_include <pybind11/detail/descr.h>

namespace upd {
namespace py {
namespace detail {

constexpr char pykey_prefix[] = "__";

template<typename Keyring, std::size_t I>
const char *cstr_name(const std::string &name) {
  static std::string static_name{pykey_prefix + name};

  return static_name.c_str();
}

template<typename Keyring, typename Index_T, Index_T I, typename R, typename... Args, endianess E, signed_mode S>
void define_pykey(pybind11::module &pymodule, const std::string &name, Keyring, key<Index_T, I, R(Args...), E, S> k) {
  using key_t = decltype(k);

  auto *pyclass_name = cstr_name<Keyring, I>(name);

  pybind11::class_<key_t>{pymodule, pyclass_name}
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

  pymodule.attr(pyclass_name + 2) = k;
}

template<typename Keyring, typename Vector, std::size_t... Is>
void define_pykeys(pybind11::module &pymodule, Keyring keyring, const Vector &matches, std::index_sequence<Is...>) {
  using flist_t = typename Keyring::flist_t;

  using discard = int[];
  discard{0, (define_pykey(pymodule, matches[Is][1], keyring, keyring.get(upd::detail::at<flist_t, Is>{})), 0)...};
}

template<typename T>
auto ensure_class(pybind11::module &pymodule, const char *name) {
  return !pybind11::hasattr(pymodule, name) ? std::optional{pybind11::class_<T>{pymodule, name}} : std::nullopt;
}

template<typename T>
auto ensure_enum(pybind11::module &pymodule, const char *name) {
  return !pybind11::hasattr(pymodule, name) ? std::optional{pybind11::enum_<T>{pymodule, name}} : std::nullopt;
}

} // namespace detail

template<typename Keyring, UPD_REQUIREMENT(is_keyring, Keyring)>
void unpack_keyring(pybind11::module &pymodule, Keyring keyring) {
  int status;
  auto *cstr_name = abi::__cxa_demangle(typeid(keyring).name(), NULL, NULL, &status);

  if (!cstr_name)
    throw std::runtime_error{(std::string) "`Keyring` identifier (" + typeid(keyring).name() +
                             ") couldn't be demangled"};

  std::string name{cstr_name};
  free(cstr_name);

  std::regex callback_name_re{"upd::unevaluated<[^,]*,\\s(\\w+)[^>]*>"};
  std::sregex_iterator begin{name.begin(), name.end(), callback_name_re}, end;
  std::vector matches(begin, end);

  if (matches.size() != keyring.size) {
    std::string msg{"Some callback identifiers could not have been extracted from `Keyring` demangled identifier or "
                    "too many of them were found. The found identifiers were : \n"};
    for (auto &match : matches)
      msg += (std::string) "- " + match[1].str() + ";\n";

    msg += (std::string) "The `Keyring` identifier was : " + name;

    throw std::invalid_argument(msg);
  }

  detail::define_pykeys(pymodule, keyring, matches, std::make_index_sequence<Keyring::size>{});
}

template<typename Keyring, UPD_REQUIREMENT(is_keyring, Keyring)>
void declare_dispatcher(pybind11::module &pymodule, const char *name, Keyring keyring) {
  using namespace pybind11::literals;
  using dispatcher_t = decltype(double_buffered_dispatcher{Keyring{}, policy::any_action});

  detail::ensure_class<action>(pymodule, "_Action");
  if (auto opt = detail::ensure_enum<packet_status>(pymodule, "PacketStatus"))
    opt.value()
        .value("LOADING_PACKET", packet_status::LOADING_PACKET)
        .value("DROPPED_PACKET", packet_status::DROPPED_PACKET)
        .value("RESOLVED_PACKET", packet_status::RESOLVED_PACKET);

  pybind11::class_<dispatcher_t>{pymodule, name}
      .def(pybind11::init([]() { return dispatcher_t{}; }))
      .def("resolve",
           [](dispatcher_t &self, pybind11::bytes bytes) {
             std::vector<byte_t> retval;
             auto it = bytes.begin();

             self(
                 [&]() {
                   ++it;
                   return it->cast<byte_t>();
                 },
                 [&](byte_t b) { retval.push_back(b); });

             // Have to perform an eval because direct conversion with `pybind11::bytes` raise a segfault
             return pybind11::eval("bytes(retval)", pybind11::dict{"retval"_a = retval});
           })
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
