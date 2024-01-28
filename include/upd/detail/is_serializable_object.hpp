#pragma once

#include <type_traits>

#include "type_traits/remove_cv_ref.hpp"

namespace upd::detail {

template<typename, typename, typename = void>
struct is_serializable_object : std::false_type {};

template<typename T, typename Serializer_T>
struct is_serializable_object<T,
                              Serializer_T,
                              std::void_t<decltype(&remove_cv_ref_t<Serializer_T>::template deserialize_object<T>)>>
    : std::true_type {};

template<typename T, typename Serializer_T>
constexpr inline bool is_serializable_object_v = is_serializable_object<T, Serializer_T>::value;

} // namespace upd::detail
