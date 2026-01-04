#pragma once

#include <tuple>
#include <type_traits>

#include "../get.hpp"
#include "../tuple/apply.hpp"
#include "../tuple/filter.hpp"
#include "../tuple/to.hpp"
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

  auto dependent_equations = sys | updv::filter([&]<typename Eq>(typebox<Eq>) {
                               return depends_on_v<typename std::remove_cvref_t<Eq>::lhs_type, Varname> ||
                                      depends_on_v<typename std::remove_cvref_t<Eq>::rhs_type, Varname>;
                             }) |
                             updv::to<std::tuple>;

  auto lets = sys |
              updv::filter([]<typename Eq>(typebox<Eq>) { return variadic::is_template_deductible_from<let, Eq>(); }) |
              updv::to<std::tuple>;

  auto dependent_lets =
      dependent_equations |
      updv::filter([]<typename Eq>(typebox<Eq>) { return variadic::is_template_deductible_from<let, Eq>(); }) |
      updv::to<std::tuple>;

  if constexpr (tuple_size_v<decltype(dependent_lets)> > 0) {
    return get<0>(dependent_lets).rhs;
  } else {
    auto eq = get<0>(dependent_equations);
    return updv::apply(lets, [&](const auto &...ls) { return side{eq.isolate(var).rhs}.calculate(ls...); });
  }
}

} // namespace upd::algebra
