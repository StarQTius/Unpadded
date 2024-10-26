#pragma once

#include <type_traits>

namespace upd {

template<typename T>
concept metatype = requires { typename T::type; };

template<typename T>
concept metavalue = requires { T::value; };

template<typename T, template<typename> typename Base>
concept implementer = std::derived_from<T, Base<T>>;

template<typename T>
struct typebox {
  using type = T;
  using unqualified_type = std::remove_cvref_t<T>;

  constexpr auto operator->() const noexcept -> const unqualified_type * { return nullptr; }
};

} // namespace upd
