#pragma once

#include <array>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "description.hpp"
#include "detail/always_false.hpp"
#include "detail/integral_constant.hpp"
#include "detail/tuple_operations.hpp"
#include "detail/variadic/concat.hpp"
#include "detail/variadic/find.hpp"
#include "detail/variadic/count.hpp"
#include "integer.hpp"
#include "upd.hpp"

namespace upd {

template<typename>
struct kw_t;

template<auto..., typename... Ts>
[[nodiscard]] constexpr auto name_tuple(std::tuple<Ts...>) noexcept;

template<typename... Named_Tuple_Ts>
[[nodiscard]] constexpr auto concat_named_tuple(Named_Tuple_Ts &&...);

template<typename Tuple_T, auto... Ids>
class named_tuple {
  using id_ts = detail::integral_constant_tuple_t<Ids...>;

  template<auto..., typename... Ts>
  friend constexpr auto name_tuple(std::tuple<Ts...>) noexcept;

  template<typename... Named_Tuple_Ts>
  friend constexpr auto concat_named_tuple(Named_Tuple_Ts &&...);

public:
  template<auto Id>
  [[nodiscard]] constexpr auto get() noexcept -> auto & {
    using id_t = detail::integral_constant_t<Id>;
    constexpr auto id_pos = detail::variadic::find_v<id_ts, id_t>;

    static_assert(id_pos != detail::variadic::not_found, "`Id` does not belong to `Ids`");

    return std::get<id_pos>(m_content);
  }

  template<auto Id>
  [[nodiscard]] constexpr auto get() const noexcept -> const auto & {
    using id_t = detail::integral_constant_t<Id>;
    constexpr auto id_pos = detail::variadic::find_v<id_ts, id_t>;

    static_assert(id_pos != detail::variadic::not_found, "`Id` does not belong to `Ids`");

    return std::get<id_pos>(m_content);
  }

  template<typename Identifier>
  [[nodiscard]] constexpr auto operator[](kw_t<Identifier>) noexcept(release) -> auto & {
    return get<Identifier::value>();
  }

  template<typename Identifier>
  [[nodiscard]] constexpr auto operator[](kw_t<Identifier>) const noexcept(release) -> const auto & {
    return get<Identifier::value>();
  }

  template<typename Serializer, typename OutputIt>
  constexpr void serialize(Serializer &ser, OutputIt output)  {
    auto serialize_pack = [&] (const auto &... xs) {
      (xs.serialize(ser, output), ...);
    };

    std::apply(serialize_pack, m_content);
  }

private:
  constexpr explicit named_tuple(Tuple_T content, detail::integral_constant_tuple_t<Ids...>)
      : m_content{UPD_FWD(content)} {}

  Tuple_T m_content;
};

template<typename>
struct is_named_tuple_instance: std::false_type {};

template<typename Tuple, auto... Identifiers>
struct is_named_tuple_instance<named_tuple<Tuple, Identifiers...>>: std::true_type {};

template<auto... Ids, typename... Ts>
[[nodiscard]] constexpr auto name_tuple(std::tuple<Ts...> tuple) noexcept {
  using id_ts = detail::integral_constant_tuple_t<Ids...>;

  if constexpr (sizeof...(Ids) != sizeof...(Ts)) {
    static_assert(UPD_ALWAYS_FALSE, "`Ids` and `Ts` must contain the same number of elements");
  } else if constexpr (((detail::variadic::count_v<id_ts, detail::integral_constant_t<Ids>> != 1) || ...)) {
    static_assert(UPD_ALWAYS_FALSE, "Each element in `Ids` must be unique");
  } else {
    return named_tuple{std::move(tuple), id_ts{}};
  }
}

template<typename... Named_Tuple_Ts>
[[nodiscard]] constexpr auto concat_named_tuple(Named_Tuple_Ts &&...named_tuples) {
  using id_ts = detail::variadic::concat_t<typename Named_Tuple_Ts::id_ts...>;

  auto content = std::tuple_cat(UPD_FWD(named_tuples).m_content...);
  auto name_content = [&](auto... id_iconsts) { return name_tuple<id_iconsts.value...>(std::move(content)); };

  return std::apply(name_content, id_ts{});
}

} // namespace upd