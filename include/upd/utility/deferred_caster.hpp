#pragma once

#include <concepts>
#include <tuple>
#include <type_traits>
#include <variant>

#include "../record.hpp"
#include "../tuple/find.hpp"
#include "../upd.hpp"
#include "is_instance_of.hpp"
#include "static_assert.hpp"

namespace upd {

template<typename... Targets>
class deferred_caster {
  template<typename Orig, typename Target>
  static auto caster(const void *orig) noexcept(release) -> Target {
    const auto &o = *reinterpret_cast<const Orig *>(orig);
    auto cast_alt = []<typename T>(const T &alt) -> Target {
      if constexpr (std::constructible_from<Target, T>) {
        return Target{alt};
      } else {
        UPD_ASSERT(false);
      }
    };

    if constexpr (std::constructible_from<Target, Orig>) {
      return Target{o};
    }

    if constexpr (is_instance_of<Orig, std::variant>()) {
      return std::visit(cast_alt, o);
    }

    if constexpr (std::is_null_pointer_v<Orig>
                  && std::default_initializable<Target>) {
      return Target{};
    }

    UPD_ASSERT(false);
  }

public:
  constexpr deferred_caster() noexcept(release) : deferred_caster{nullptr} {}

  template<typename T>
  constexpr deferred_caster(const T &orig) noexcept(release)
      : m_orig{&orig}, m_casters{caster<T, Targets>...} {}

  template<typename Target>
  [[nodiscard]] constexpr auto cast_to() const noexcept(release) {
    using namespace tuple_views;

    UPD_ASSERT(m_orig);

    auto i = find_if(m_casters, []<typename F> {
      return std::is_invocable_r_v<Target, F, const void *>;
    });
    UPD_STATIC_ASSERT(i < sizeof...(Targets), "Cannot cast to target type `{}`",
                      typebox<Target>{});

    return UPD_INVOKE(get<i>(m_casters), m_orig);
  }

private:
  const void *m_orig;
  std::tuple<Targets (*)(const void *)...> m_casters;
};

} // namespace upd
