#pragma once

#include <type_traits>

namespace upd::detail {

template<typename, typename = void>
struct has_value_member : std::false_type {};

template<typename T>
struct has_value_member<T, std::void_t<decltype(T::value)>> : std::true_type {};

template<typename T>
constexpr auto has_value_member_v = has_value_member<T>::value;

} // namespace upd::detail