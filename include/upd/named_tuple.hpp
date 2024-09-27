#pragma once

#include <array>
#include <ranges>
#include <string>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "description.hpp"
#include "detail/always_false.hpp"
#include "detail/integral_constant.hpp"
#include "detail/tuple_operations.hpp"
#include "detail/variadic/concat.hpp"
#include "detail/variadic/count.hpp"
#include "detail/variadic/find.hpp"
#include "integer.hpp"
#include "upd.hpp"

namespace upd {

template<std::size_t N>
struct name {
  constexpr name(const char (&s)[N]): value{} {
    std::ranges::copy(s, value);
  }

  template<std::size_t M>
  [[nodiscard]] constexpr auto operator==(const name<M> &rhs) const noexcept -> bool {
    return std::string_view{value} == std::string_view{rhs.value};
  };

  char value[N];
};

template<name>
struct kw_t;

template<name..., typename... Ts>
[[nodiscard]] constexpr auto name_tuple(std::tuple<Ts...>) noexcept;

template<typename... Named_Tuple_Ts>
[[nodiscard]] constexpr auto concat_named_tuple(Named_Tuple_Ts &&...);

template<typename Tuple_T, name... Ids>
class named_tuple {
  using id_ts = detail::integral_constant_tuple_t<Ids...>;

  template<name..., typename... Ts>
  friend constexpr auto name_tuple(std::tuple<Ts...>) noexcept;

  template<typename... Named_Tuple_Ts>
  friend constexpr auto concat_named_tuple(Named_Tuple_Ts &&...);

public:
  template<name Id>
  [[nodiscard]] constexpr auto get() noexcept -> auto & {
    using id_t = detail::integral_constant_t<Id>;
    constexpr auto id_pos = detail::variadic::find_v<id_ts, id_t>;

    static_assert(id_pos != detail::variadic::not_found, "`Id` does not belong to `Ids`");

    return std::get<id_pos>(m_content);
  }

  template<name Id>
  [[nodiscard]] constexpr auto get() const noexcept -> const auto & {
    using id_t = detail::integral_constant_t<Id>;
    constexpr auto id_pos = detail::variadic::find_v<id_ts, id_t>;

    static_assert(id_pos != detail::variadic::not_found, "`Id` does not belong to `Ids`");

    return std::get<id_pos>(m_content);
  }

  template<name Identifier>
  [[nodiscard]] constexpr auto operator[](kw_t<Identifier>) noexcept(release) -> auto & {
    return get<Identifier>();
  }

  template<name Identifier>
  [[nodiscard]] constexpr auto operator[](kw_t<Identifier>) const noexcept(release) -> const auto & {
    return get<Identifier>();
  }

  template<typename Serializer, typename OutputIt>
  constexpr void serialize(Serializer &ser, OutputIt output) {
    auto serialize_pack = [&](const auto &...xs) { (xs.serialize(ser, output), ...); };

    std::apply(serialize_pack, m_content);
  }

private:
  constexpr explicit named_tuple(Tuple_T content, detail::integral_constant_tuple_t<Ids...>)
      : m_content{UPD_FWD(content)} {}

  Tuple_T m_content;
};

template<typename>
struct is_named_tuple_instance : std::false_type {};

template<typename Tuple, name... Identifiers>
struct is_named_tuple_instance<named_tuple<Tuple, Identifiers...>> : std::true_type {};

template<name... Ids, typename... Ts>
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
