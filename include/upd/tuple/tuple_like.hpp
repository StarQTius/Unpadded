#pragma once

#include <concepts>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../implementation_of.hpp"
#include "../record/record_element.hpp"
#include "../record/record_size.hpp"
#include "../record/record_tag.hpp"
#include "../upd.hpp"
#include "../variadic_concept.hpp"

namespace upd {

template<typename Tuple, std::size_t I>
concept ith_tuple_element_gettable = requires(std::remove_reference_t<Tuple> x) {
  typename std::tuple_element_t<I, decltype(x)>;
} && requires(Tuple &&t) {
  { get<I>(UPD_FWD(t)) } -> std::common_reference_with<std::tuple_element_t<I, std::remove_reference_t<Tuple>>>;
};

template<typename Tuple>
concept tuple_like2 = requires(std::remove_reference_t<Tuple> x) {
  std::tuple_size<decltype(x)>::value;
  { std::tuple_size_v<decltype(x)> } -> std::convertible_to<std::size_t>;
} && UPD_ALL_OF_CONCEPT(ith_tuple_element_gettable, Tuple, std::tuple_size_v<std::remove_reference_t<Tuple>>);

template<typename>
struct tuple_like_for; // IWYU pragma: keep

template<std::size_t I, typename Tuple>
  requires implementation_of<Tuple, tuple_like_for>
[[nodiscard]] constexpr auto get(Tuple &&t) -> decltype(auto) {
  using impl_type = tuple_like_for<std::remove_cvref_t<Tuple>>;
  return impl_type::template get<I>(UPD_FWD(t));
}

using std::get;

} // namespace upd

template<typename Tuple>
  requires upd::implementation_of<Tuple, upd::tuple_like_for>
struct std::tuple_size<Tuple> {
  constexpr static auto value = upd::tuple_like_for<std::remove_cvref_t<Tuple>>::size;
};

template<std::size_t I, typename Tuple>
  requires upd::implementation_of<Tuple, upd::tuple_like_for>
struct std::tuple_element<I, Tuple> {
  using type = typename upd::tuple_like_for<std::remove_cvref_t<Tuple>>::template element_type<I>;
};

template<upd::tuple_like2 Tuple>
  requires(!(std::is_reference_v<Tuple> || std::is_const_v<Tuple>))
struct upd::record_size<Tuple> {
  constexpr static auto value = std::tuple_size_v<Tuple>;
};

template<std::size_t I, upd::tuple_like2 Tuple>
  requires(!(std::is_reference_v<Tuple> || std::is_const_v<Tuple>) && I < std::tuple_size_v<Tuple>)
struct upd::record_tag<I, Tuple> {
  constexpr static auto value = I;
};

template<std::size_t I, upd::tuple_like2 Tuple>
  requires(!(std::is_reference_v<Tuple> || std::is_const_v<Tuple>) && I < std::tuple_size_v<Tuple>)
struct upd::record_element<I, Tuple> {
  using type = std::tuple_element_t<I, Tuple>;
};
