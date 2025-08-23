#pragma once

#include <concepts>
#include <type_traits>

#include "upd.hpp"

namespace upd {

template<typename T>
concept metatype = requires { typename T::type; };

template<typename T>
concept metavalue = std::is_empty_v<T> && requires { T::value; };

template<typename T, template<typename> typename Base>
concept implementer = std::derived_from<T, Base<T>>;

template<typename T>
struct typebox {
  using type = T;
  using unqualified_type = std::remove_cvref_t<T>;

  template<template<typename, typename...> typename Predicate, typename... Args>
    requires metavalue<Predicate<T, Args...>>
  [[nodiscard]] constexpr static auto satisfies() noexcept(release) -> bool {
    return Predicate<T, Args...>::value;
  }

  constexpr auto operator->() const noexcept -> const unqualified_type * { return nullptr; }
};

} // namespace upd
