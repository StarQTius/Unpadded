#pragma once

#include <cstddef>

#include "always_false.hpp"
#include "integral_constant.hpp"
#include "is_serializable_object.hpp" // IWYU pragma: keep
#include "type_traits/is_array.hpp"
#include "type_traits/remove_cv_ref.hpp"
#include "type_traits/signature.hpp"
#include "variadic/map.hpp"
#include "variadic/sum.hpp" // IWYU pragma: keep

namespace upd::detail {

template<typename T, typename Serializer_T>
[[nodiscard]] constexpr auto serialized_size() noexcept -> std::size_t {
  if constexpr (std::is_integral_v<T> || detail::is_array<T>::value) {
    return sizeof(T);
  } else if constexpr (is_serializable_object_v<T, Serializer_T>) {
    auto f = [](auto x) { return integral_constant_t<serialized_size<decltype(x), Serializer_T>()>{}; };

    using deserialize_t = decltype(&detail::remove_cv_ref_t<Serializer_T>::template deserialize_object<T>);
    using args_t = typename detail::examine_invocable<deserialize_t>::args;
    using sizes_t = variadic::mapf_t<args_t, decltype(f)>;
    return variadic::sum_v<sizes_t>;
  } else {
    static_assert(UPD_ALWAYS_FALSE, "Type is not serializable");
  }
}

} // namespace upd::detail
