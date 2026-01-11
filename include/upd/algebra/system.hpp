#pragma once

#include <tuple>
#include <type_traits>

#include "../equivalent_to.hpp"
#include "../get.hpp"
#include "../is_instance_of.hpp"
#include "../tuple/apply.hpp"
#include "../tuple/concat.hpp"
#include "../tuple/filter.hpp"
#include "../tuple/to.hpp"
#include "../tuple/transform.hpp"
#include "../tuple/tuple_like.hpp"
#include "../tuple/tuple_size.hpp"
#include "../type_traits.hpp"
#include "../upd.hpp"
#include "../variadic/is_template_deductible_from.hpp"
#include "concepts.hpp"
#include "let.hpp"
#include "side.hpp"
#include "variable.hpp"

namespace upd::algebra {

template<auto Varname, tuple_like2 System>
[[nodiscard]] constexpr auto solve_for(side<variable<Varname>> var, const System &sys) noexcept(release) {
  namespace updv = upd::tuple_views;

  auto sys2 = sys | updv::filter([&]<typename Eq>(typebox<Eq>) {
                using lhs_type = typename std::remove_cvref_t<Eq>::lhs_type;
                using rhs_type = typename std::remove_cvref_t<Eq>::rhs_type;
                if constexpr (!is_instance_of<lhs_type, variable>() || !is_instance_of<rhs_type, variable>()) {
                  return true;
                } else {
                  return !equivalent_to<lhs_type::name, rhs_type::name>;
                }
              }) |
              updv::to<std::tuple>;

  auto lets = sys2 |
              updv::filter([]<typename Eq>(typebox<Eq>) { return variadic::is_template_deductible_from<let, Eq>(); }) |
              updv::to<std::tuple>;

  auto sys3_ =
      sys2 | updv::filter([]<typename Eq>(typebox<Eq>) { return !variadic::is_template_deductible_from<let, Eq>(); }) |
      updv::transform(
          [&](const auto &eq) { return updv::apply(lets, [&](auto... ls) { return eq.substitute(ls...); }); }) |
      updv::transform([&](const auto &eq) { return eq.simplify(); }) | updv::to<std::tuple>;

  auto sys3 = updv::concat(lets, sys3_) | updv::to<std::tuple>;

  auto dependent_equations = sys3 | updv::filter([&]<typename Eq>(typebox<Eq>) {
                               return depends_on_v<typename std::remove_cvref_t<Eq>::lhs_type, Varname> ||
                                      depends_on_v<typename std::remove_cvref_t<Eq>::rhs_type, Varname>;
                             }) |
                             updv::to<std::tuple>;

  auto dependent_lets =
      dependent_equations |
      updv::filter([]<typename Eq>(typebox<Eq>) { return variadic::is_template_deductible_from<let, Eq>(); }) |
      updv::transform([](auto eq) { return let{eq}; }) | updv::to<std::tuple>;

  auto lets_ = sys3 |
               updv::filter([]<typename Eq>(typebox<Eq>) { return variadic::is_template_deductible_from<let, Eq>(); }) |
               updv::to<std::tuple>;

  // int x = sys3;

  if constexpr (tuple_size_v<decltype(dependent_lets)> > 0) {
    return get<0>(dependent_lets).val;
  } else {
    auto eq = get<0>(dependent_equations);
    return updv::apply(lets_, [&](const auto &...ls) { return side{eq.isolate(var).rhs}.calculate(ls...); });
  }
}

} // namespace upd::algebra
