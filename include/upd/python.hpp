#pragma once

#include <cstring>
#include <regex>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include <boost/core/demangle.hpp>
#include <pybind11/cast.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include "detail/type_traits/require.hpp"
#include "key.hpp"
#include "upd.hpp"
#include "upd/detail/type_traits/typelist.hpp"
#include "upd/format.hpp"
#include "upd/type.hpp"

#if !defined(UPD_IDENTIFIER_LENGTH_LIMIT)
#define UPD_IDENTIFIER_LENGTH_LIMIT 64
#endif

namespace upd {
namespace py {
namespace detail {

constexpr std::size_t identifier_length_limit = UPD_IDENTIFIER_LENGTH_LIMIT;
constexpr char pykey_prefix[] = "__";

template<typename Keyring, std::size_t I>
const char *cstr_name(const std::string &name) {
  static bool is_name_set;
  static char cstr_name[identifier_length_limit];

  if (!is_name_set) {
    if (!(name.size() < identifier_length_limit - sizeof pykey_prefix))
      throw std::length_error(
          (std::string) "Identifier `" + name + "` length is above the supported limit (" +
          std::to_string(identifier_length_limit) +
          "). If you need longer identifier, increase the limit by setting the `UPD_IDENTIFIER_LENGTH_LIMIT` macro");
    std::strcpy(cstr_name, pykey_prefix);
    std::strcpy(cstr_name + sizeof pykey_prefix - 1, name.c_str());
    is_name_set = true;
  }

  return cstr_name;
}

template<typename Keyring, typename Index_T, Index_T I, typename R, typename... Args, endianess E, signed_mode S>
void define_pykey(pybind11::module &pymodule, const std::string &name, Keyring, key<Index_T, I, R(Args...), E, S> k) {
  using key_t = decltype(k);

  auto *pyclass_name = cstr_name<Keyring, I>(name);

  pybind11::class_<key_t>{pymodule, pyclass_name}
      .def("encode",
           [](key_t k, const Args &...args) {
             byte_t buf[key_t::payload_length];

             k(args...).write_all(buf);
             return pybind11::bytes{reinterpret_cast<char *>(buf), sizeof buf};
           })
      .def("decode", [](key_t k, pybind11::object &pybytes) {
        auto it = pybytes.begin();
        return k.read_all([&]() {
          ++it;
          return it->cast<byte_t>();
        });
      });

  pymodule.attr(pyclass_name + 2) = k;
}

template<typename Keyring, typename Vector, std::size_t... Is>
void define_pykeys(pybind11::module &pymodule, Keyring keyring, const Vector &matches, std::index_sequence<Is...>) {
  using flist_t = typename Keyring::flist_t;

  using discard = int[];
  discard{0, (define_pykey(pymodule, matches[Is][1], keyring, keyring.get(upd::detail::at<flist_t, Is>{})), 0)...};
}

} // namespace detail

template<typename Keyring, UPD_REQUIREMENT(is_keyring, Keyring)>
auto bind(pybind11::module &pymodule, Keyring keyring) {
  auto name = boost::core::demangle(typeid(keyring).name());
  std::regex callback_name_re{"upd::unevaluated<[^,]*,\\s(\\w+)[^>]*>"};
  std::sregex_iterator begin{name.begin(), name.end(), callback_name_re}, end;
  std::vector matches(begin, end);

  if (matches.size() != keyring.size) {
    std::string msg{"Some callback identifiers could not have been extracted from `Keyring` demangled identifier or "
                    "too many of them were found. The found identifiers were : \n"};
    for (auto &match : matches)
      msg += (std::string) "- " + match[1].str() + ";\n";

    throw std::invalid_argument(msg);
  }

  detail::define_pykeys(pymodule, keyring, matches, std::make_index_sequence<Keyring::size>{});
}

} // namespace py
} // namespace upd
