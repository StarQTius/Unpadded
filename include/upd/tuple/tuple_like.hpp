#pragma once

#include <concepts>
#include <cstddef>
#include <format>
#include <type_traits>
#include <utility>

#include "../upd.hpp"
#include "../utility/get.hpp"
#include "../utility/implementation_of.hpp"
#include "../utility/variadic_concept.hpp"
#include "../utility/with_sequence.hpp"
#include "tuple_size.hpp"

namespace upd {

template<typename Tuple, std::size_t I>
concept ith_tuple_element_gettable = requires(std::remove_reference_t<Tuple> x) {
  typename std::tuple_element_t<I, decltype(x)>;
} && requires(Tuple &&t) {
  { get<I>(UPD_FWD(t)) } -> std::common_reference_with<std::tuple_element_t<I, std::remove_reference_t<Tuple>>>;
};

template<typename Tuple>
concept tuple_like2 = requires(std::remove_cvref_t<Tuple> x) {
  std::tuple_size<decltype(x)>::value;
  { std::tuple_size_v<decltype(x)> } -> std::convertible_to<std::size_t>;
} && UPD_ALL_OF_CONCEPT(ith_tuple_element_gettable, Tuple, std::tuple_size_v<std::remove_reference_t<Tuple>>);

template<typename>
struct tuple_like_for; // IWYU pragma: keep

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

template<upd::tuple_like2 TupleLike>
struct std::formatter<TupleLike> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  template<typename Tuple>
  static auto format(const Tuple &t, std::format_context &ctx) {
    auto it = ctx.out();
    it = std::format_to(it, "(");

    auto first = true;
    UPD_WITH_SEQUENCE(Is, upd::tuple_size_v<Tuple>, &) {
      auto format_elem = [&](const auto &e) {
        if (first) {
          it = std::format_to(it, "{}", e);
          first = false;
        } else {
          it = std::format_to(it, ", {}", e);
        }
      };

      (format_elem(upd::get<Is>(t)), ...);
    };
    it = std::format_to(it, ")");

    ctx.advance_to(it);
    return it;
  }
};
