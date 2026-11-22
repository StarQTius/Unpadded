#pragma once

#include <concepts>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <variant>
#include <version>

#include "../record.hpp"
#include "../tuple/find.hpp"
#include "../upd.hpp"
#include "static_assert.hpp"

namespace upd {

template<typename Orig, typename Target>
[[nodiscard]] constexpr auto
restore_and_cast(const void *orig) noexcept(release) -> Target {
  if constexpr (std::is_null_pointer_v<Orig>
                && std::default_initializable<Target>) {
    return Target{};
  }

  UPD_ASSERT(orig);

  const auto &o = *reinterpret_cast<const Orig *>(orig);
  if constexpr (std::convertible_to<Orig, Target>) {
    return static_cast<Target>(o);
  }

  UPD_ASSERT(false);
}

template<typename... Targets>
class deferred_caster {
public:
  constexpr deferred_caster() noexcept(release)
      : m_orig{nullptr},
        m_casters{&restore_and_cast<std::nullptr_t, Targets>...} {}

  template<typename T>
    requires(std::convertible_to<T, Targets> || ...)
  constexpr deferred_caster(const T &orig) noexcept(release)
      : m_orig{&orig}, m_casters{&restore_and_cast<T, Targets>...} {}

  template<typename... Ts>
  constexpr deferred_caster(const std::variant<Ts...> &orig) noexcept(release)
      : m_orig{nullptr} {
    std::visit(
        [this]<typename T>(const T &o) {
          m_orig = &o;
          m_casters = std::tuple{&restore_and_cast<T, Targets>...};
        },
        orig);
  }

  template<typename Target>
  [[nodiscard]] constexpr auto cast_to() const noexcept(release) {
    using namespace tuple_views;

    auto i = find_if(m_casters, []<typename F> {
      return std::is_invocable_r_v<Target, F, const void *>;
    });
    UPD_STATIC_ASSERT(i < sizeof...(Targets), "Cannot cast to target type `{}`",
                      typebox<Target>{});

    return UPD_INVOKE(get<i>(m_casters), m_orig);
  }

  [[nodiscard]] constexpr auto
  origin() const noexcept(release) -> const void * {
    return m_orig;
  }

private:
  const void *m_orig;
  std::tuple<Targets (*)(const void *)...> m_casters;
};

template<typename... Targets>
class deferred_range_caster {
public:
  constexpr deferred_range_caster() noexcept(release)
      : m_data{nullptr}, m_size{0}, m_element_size{0} {}

  template<std::ranges::range Range>
    requires std::ranges::contiguous_range<Range>
  explicit constexpr deferred_range_caster(const Range &r)
      : m_data{std::data(r)}, m_size{std::size(r)},
        m_element_size{sizeof(std::ranges::range_value_t<Range>)},
        m_casters{
            &restore_and_cast<std::ranges::range_value_t<Range>, Targets>...} {}

  constexpr deferred_range_caster(const deferred_range_caster &) noexcept(
      release) = default;

  constexpr deferred_range_caster(deferred_range_caster &&) noexcept(release) =
      default;

  constexpr auto operator=(const deferred_range_caster &) noexcept(release)
      -> deferred_range_caster & = default;

  constexpr auto operator=(deferred_range_caster &&) noexcept(release)
      -> deferred_range_caster & = default;

  template<typename Target>
  [[nodiscard]] constexpr auto cast_to() const noexcept(release) {
    using namespace std::views;

    auto i = upd::tuple_views::find_if(m_casters, []<typename F> {
      return std::is_invocable_r_v<Target, F, const void *>;
    });
    UPD_STATIC_ASSERT(i < sizeof...(Targets), "Cannot cast to target type `{}`",
                      typebox<Target>{});

    const auto &caster = get<i>(m_casters);
    return iota(0uz, m_size)
           | transform([&, m_data = m_data, m_esize = m_element_size](auto i) {
               const auto *orig = (const char *)m_data + i * m_esize;
               return UPD_INVOKE(caster, orig);
             });
  }

  [[nodiscard]] constexpr auto data() const noexcept(release) -> const void * {
    return m_data;
  }

private:
  const void *m_data;
  std::size_t m_size;
  std::size_t m_element_size;
  std::tuple<Targets (*)(const void *)...> m_casters;
};

} // namespace upd
