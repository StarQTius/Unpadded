#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "../record/record.hpp"
#include "../tuple/concat.hpp"
#include "../tuple/filter.hpp"
#include "../tuple/find.hpp"
#include "../tuple/to.hpp"
#include "../tuple/transform.hpp"
#include "../tuple/tuple_like.hpp"
#include "../tuple/tuple_size.hpp"
#include "../upd.hpp"
#include "../utility/always_false.hpp"
#include "../utility/constexpr.hpp"
#include "../utility/get.hpp"
#include "../utility/static_assert.hpp"
#include "../variadic/is_template_deductible_from.hpp"
#include "let.hpp"
#include "side.hpp"
#include "variable.hpp"

namespace upd {

constexpr struct unit_t {
  constexpr unit_t() noexcept(release) = default;

  constexpr unit_t(const unit_t &) noexcept(release) = default;

  template<typename T>
  constexpr unit_t(const T &) noexcept(release) {}

  constexpr unit_t &operator=(const unit_t &) noexcept(release) = default;

  template<typename T>
  constexpr unit_t &operator=(const T &) noexcept(release) {
    return *this;
  }
} unit;

template<typename T>
[[nodiscard]] constexpr auto
operator==(unit_t, const T &) noexcept(release) -> bool {
  return false;
}

template<typename T>
[[nodiscard]] constexpr auto
operator==(const T &, unit_t) noexcept(release) -> bool {
  return false;
}

[[nodiscard]] constexpr inline auto
operator==(unit_t, unit_t) noexcept(release) -> bool {
  return true;
}

} // namespace upd

namespace upd::algebra {

template<std::size_t MaxPassCount = 16, auto Varname, tuple_like2 System>
[[nodiscard]] constexpr auto
try_solve_for(side<variable<Varname>> var,
              const System &sys) noexcept(release) {
  namespace updv = upd::tuple_views;

  auto solpos = updv::find_if(sys, [&]<typename Eq> {
    using eq_type = std::remove_cvref_t<Eq>;
    return variadic::is_template_deductible_from<let, eq_type>()
           && eq_type::depends_on(var.expr);
  });

  if constexpr (solpos < tuple_size_v<decltype(sys)>) {
    return get<solpos>(sys).rhs;
  } else if constexpr (MaxPassCount > 0) {
    auto ssys =
        sys | updv::transform([&](const auto &eq) { return eq.simplify(); });
    auto lets = ssys
                | updv::filter([]<typename Eq> {
                    return variadic::is_template_deductible_from<let, Eq>();
                  })
                | updv::transform([](const auto &eq) { return let{eq}; })
                | updv::transform([](auto lt) {
                    return side{variable<lt.varname>{}} = lt.val;
                  })
                | updv::to<std::tuple>;
    auto eqs = ssys
               | updv::filter([]<typename Eq> {
                   return !variadic::is_template_deductible_from<let, Eq>();
                 })
               | updv::transform([&](const auto &eq) {
                   return eq.substitute(lets).simplify();
                 });

    auto newsys = updv::concat(lets, eqs) | updv::to<std::tuple>;
    return try_solve_for<MaxPassCount - 1>(var, newsys);
  } else {
    return unit;
  }
}

template<std::size_t MaxPassCount = 16, auto Varname, tuple_like2 System>
[[nodiscard]] constexpr auto
solve_for(side<variable<Varname>> var, const System &sys) noexcept(release) {
  auto retval = try_solve_for<MaxPassCount>(var, sys);
  if constexpr (retval != unit) {
    return retval;
  } else {
    UPD_STATIC_ASSERT(always_false<>, "Maximum number of passes reached");
  }
}

} // namespace upd::algebra
