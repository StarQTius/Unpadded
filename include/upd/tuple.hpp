#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "constexpr.hpp"
#include "functional.hpp"
#include "lite_tuple.hpp"
#include "transfert_reference.hpp"
#include "tuple_impl.hpp"
#include "type_traits.hpp"
#include "upd.hpp"

namespace upd {

template<typename F>
[[nodiscard]] constexpr auto to_metafunction(F f) {
  return [=]<metavalue... Metas>(Metas...) { return expr<UPD_INVOKE(f, Metas::value...)>; };
}

template<typename T, typename U>
struct reference_like {
  using type = std::remove_reference_t<T>;
};

template<typename T, typename U>
struct reference_like<T, U &> {
  using type = std::remove_reference_t<T> &;
};

template<typename T, typename U>
struct reference_like<T, U &&> {
  using type = std::remove_reference_t<T> &&;
};

template<typename T, typename U>
using reference_like_t = typename reference_like<T, U>::type;

template<typename T, typename BinaryOp>
class accumulable_t {
  template<typename _T, typename U, invocable<_T, U> _BinaryOp>
  friend constexpr auto operator,(accumulable_t<_T, _BinaryOp> &&, U &&) -> decltype(auto);

  template<typename U, typename _T, invocable<U, _T> _BinaryOp>
  friend constexpr auto operator,(U &&, accumulable_t<_T, _BinaryOp> &&) -> decltype(auto);

  template<typename _T, typename _BinaryOp>
  friend constexpr auto accumulable(_T &&, _BinaryOp &&) noexcept(release) -> accumulable_t<_T &&, _BinaryOp &&>;

  constexpr accumulable_t(T value, BinaryOp op) : m_value{UPD_FWD(value)}, m_op{UPD_FWD(op)} {}

  T m_value;
  BinaryOp m_op;
};

template<typename T, typename U, invocable<T, U> BinaryOp>
[[nodiscard]] constexpr auto operator,(accumulable_t<T, BinaryOp> &&acc, U &&x) -> decltype(auto) {
  return UPD_INVOKE(UPD_FWD(acc.m_op), UPD_FWD(acc.m_value), UPD_FWD(x));
}

template<typename U, typename T, invocable<U, T> BinaryOp>
[[nodiscard]] constexpr auto operator,(U &&x, accumulable_t<T, BinaryOp> &&acc) -> decltype(auto) {
  return UPD_INVOKE(UPD_FWD(acc.m_op), UPD_FWD(x), UPD_FWD(acc.m_value));
}

template<typename T, typename BinaryOp>
[[nodiscard]] constexpr auto accumulable(T &&x, BinaryOp &&op) noexcept(release) -> accumulable_t<T &&, BinaryOp &&> {
  return accumulable_t<T &&, BinaryOp &&>{UPD_FWD(x), UPD_FWD(op)};
}

template<std::size_t I, typename Tuple>
[[nodiscard]] constexpr auto get(Tuple &&t) noexcept(release) -> auto && {
  return UPD_FWD(t).template get<I, Tuple>();
}

} // namespace upd

namespace upd {

template<std::size_t, typename>
struct named_tuple_element;

template<std::size_t I, typename Tuple>
struct named_tuple_element<I, const Tuple> {
  using type = const typename named_tuple_element<I, Tuple>::type;
};

template<std::size_t I, typename Tuple>
struct named_tuple_element<I, volatile Tuple> {
  using type = volatile typename named_tuple_element<I, Tuple>::type;
};

template<std::size_t I, typename Tuple>
struct named_tuple_element<I, const volatile Tuple> {
  using type = const volatile typename named_tuple_element<I, Tuple>::type;
};

template<std::size_t I, typename Tuple>
using named_tuple_element_t = typename named_tuple_element<I, Tuple>::type;

template<std::size_t, typename>
struct named_tuple_identifier;

template<std::size_t I, typename Tuple>
struct named_tuple_identifier<I, const Tuple> {
  constexpr static auto value = named_tuple_identifier<I, Tuple>::value;
};

template<std::size_t I, typename Tuple>
struct named_tuple_identifier<I, volatile Tuple> {
  constexpr static auto value = named_tuple_identifier<I, Tuple>::value;
};

template<std::size_t I, typename Tuple>
struct named_tuple_identifier<I, const volatile Tuple> {
  constexpr static auto value = named_tuple_identifier<I, Tuple>::value;
};

template<std::size_t I, typename Tuple>
constexpr auto named_tuple_identifier_v = named_tuple_identifier<I, Tuple>::value;

template<typename T>
concept named_tuple_like =
    tuple_like<T> &&
    []<std::size_t... Is>(std::index_sequence<Is...>) {
      [[maybe_unused]] auto has_tuple_identifier = [](auto i) {
        return requires(std::remove_reference_t<T> x) { named_tuple_identifier<i, decltype(x)>::value; };
      };
      return (has_tuple_identifier(expr<Is>) && ...);
    }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{}) &&
    []<std::size_t... Is>(std::index_sequence<Is...>) {
      [[maybe_unused]] auto has_tuple_identifier = [](auto i) {
        return requires(std::remove_reference_t<T> x) { typename named_tuple_element<i, decltype(x)>::type; };
      };
      return (has_tuple_identifier(expr<Is>) && ...);
    }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{}) &&
    []<std::size_t... Is>(std::index_sequence<Is...>) {
      [[maybe_unused]] auto is_nth_id_gettable = []([[maybe_unused]] auto i) {
        using noref_type = std::remove_reference_t<T>;
        [[maybe_unused]] constexpr auto identifier = named_tuple_identifier_v<i, noref_type>;
        return requires(T &&x) {
          {
            get<named_tuple_identifier_v<i, noref_type>>(UPD_FWD(x))
          } -> std::same_as<transfert_reference_t<named_tuple_element_t<i, noref_type> &&, T &&>>;
        };
      };
      return (is_nth_id_gettable(expr<Is>) && ...);
    }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});

template<tuple_like Tuple>
[[nodiscard]] constexpr auto type_only(const Tuple &) noexcept(release);

template<typename... Ts>
class tuple : public tuple_implementation<tuple<Ts...>> {
  using leaves = detail::leaves<std::index_sequence_for<Ts...>, Ts...>;

public:
  template<typename... Us, typename... Args>
  [[nodiscard]] constexpr static auto make_tuple(typelist<Us...>, Args &&...xs) {
    return tuple<Us...>{std::in_place, UPD_FWD(xs)...};
  }

  template<typename... Us>
  [[nodiscard]] constexpr static auto make_typelist(typelist<Us...>) noexcept(release) {
    return typelist<Us...>{};
  }

  template<auto... Values>
  [[nodiscard]] constexpr static auto make_constlist(constlist<Values...>) noexcept(release) {
    return constlist<Values...>{};
  }

  constexpr tuple() = default;

  constexpr tuple(const tuple &) = default;

  template<typename... Us>
    requires(sizeof...(Ts) == sizeof...(Us))
  constexpr tuple(const tuple<Us...> &other) : m_leaves{other} {}

  constexpr tuple(tuple &&) = default;

  template<typename... Us>
    requires(sizeof...(Ts) == sizeof...(Us))
  constexpr tuple(tuple &&other) : m_leaves{std::move(other)} {}

  constexpr tuple &operator=(const tuple &) = default;

  constexpr tuple &operator=(tuple &&) = default;

  template<typename... Us>
    requires(sizeof...(Us) > 1 || !(tuple_like<Us> && ...))
  constexpr tuple(Us &&...xs) : m_leaves{std::in_place, UPD_FWD(xs)...} {}

  template<typename... Us>
  constexpr explicit tuple(std::in_place_t, Us &&...xs) : m_leaves{std::in_place, UPD_FWD(xs)...} {}

  template<std::size_t I, typename Self>
  [[nodiscard]] constexpr auto get(this Self &&self) noexcept(release) -> auto && {
    decltype(auto) retval = UPD_FWD(self).m_leaves.at(expr<I>);

    return UPD_FWD(retval);
  }

private:
  leaves m_leaves;
};

} // namespace upd

template<typename... Ts>
struct std::tuple_size<upd::tuple<Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

template<std::size_t I, typename... Ts>
struct std::tuple_element<I, upd::tuple<Ts...>> {
  using type = typename decltype(auto{upd::detail::lite_tuple<upd::typebox<Ts>...>{}.at(upd::expr<I>)})::type;
};
