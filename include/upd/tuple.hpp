#pragma once

#include "detail/variadic/at.hpp"
#include "detail/variadic/leaf.hpp"

namespace upd {

struct encapsulate_t {};

constexpr auto encapsulate = encapsulate_t{};

template<typename Operator>
struct operator_invoker {
  constexpr static auto op = Operator{};

  template<typename Lhs, typename Rhs>
  [[nodiscard]] constexpr auto operator()(const Lhs &lhs, const Rhs &rhs) const -> decltype(auto) {
    return op(lhs, rhs);
  }

  template<typename Lhs, typename Rhs>
  [[nodiscard]] constexpr auto operator()(const std::pair<Lhs, Rhs> &lhs_rhs) const -> decltype(auto) {
    const auto &[lhs, rhs] = lhs_rhs;
    return op(lhs, rhs);
  }
};

constexpr inline auto equal_to = operator_invoker<std::equal_to<>>{};
constexpr inline auto plus = operator_invoker<std::plus<>>{};

} // namespace upd

namespace upd {

template<auto Value>
struct auto_constant {
  using type = auto_constant<Value>;
  using value_type = std::remove_cv_t<std::remove_reference_t<decltype(Value)>>;

  constexpr static auto value = [] {
    if constexpr (std::is_invocable_v<value_type>) {
      return Value();
    } else {
      return Value;
    }
  }();

  template<typename T>
  [[nodiscard]] constexpr operator T() {
    return static_cast<T>(value);
  }
};

template<auto Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator==(auto_constant<Lhs>, auto_constant<Rhs>) noexcept -> decltype(auto) {
  return auto_constant<Lhs == Rhs>{};
}

template<auto Lhs, typename Rhs>
[[nodiscard]] constexpr auto operator==(auto_constant<Lhs>, Rhs &&rhs) -> decltype(auto) {
  return Lhs == UPD_FWD(rhs);
}

template<typename Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator==(Lhs &&lhs, auto_constant<Rhs>) -> decltype(auto) {
  return UPD_FWD(lhs) == Rhs;
}

template<auto Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator<(auto_constant<Lhs>, auto_constant<Rhs>) noexcept -> decltype(auto) {
  return auto_constant < Lhs<Rhs>{};
}

template<auto Lhs, typename Rhs>
[[nodiscard]] constexpr auto operator<(auto_constant<Lhs>, Rhs &&rhs) -> decltype(auto) {
  return Lhs < UPD_FWD(rhs);
}

template<typename Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator<(Lhs &&lhs, auto_constant<Rhs>) -> decltype(auto) {
  return UPD_FWD(lhs) < Rhs;
}

template<auto Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator+(auto_constant<Lhs>, auto_constant<Rhs>) noexcept -> decltype(auto) {
  return auto_constant<Lhs + Rhs>{};
}

template<auto Lhs, typename Rhs>
[[nodiscard]] constexpr auto operator+(auto_constant<Lhs>, Rhs &&rhs) -> decltype(auto) {
  return Lhs + UPD_FWD(rhs);
}

template<typename Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator+(Lhs &&lhs, auto_constant<Rhs>) -> decltype(auto) {
  return UPD_FWD(lhs) + Rhs;
}

template<auto Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator-(auto_constant<Lhs>, auto_constant<Rhs>) noexcept -> decltype(auto) {
  return auto_constant<Lhs - Rhs>{};
}

template<auto Lhs, typename Rhs>
[[nodiscard]] constexpr auto operator-(auto_constant<Lhs>, Rhs &&rhs) -> decltype(auto) {
  return Lhs - UPD_FWD(rhs);
}

template<typename Lhs, auto Rhs>
[[nodiscard]] constexpr auto operator-(Lhs &&lhs, auto_constant<Rhs>) -> decltype(auto) {
  return UPD_FWD(lhs) - Rhs;
}

template<typename>
struct is_auto_constant : std::false_type {};

template<auto Value>
struct is_auto_constant<auto_constant<Value>> : std::true_type {};

template<typename T>
constexpr auto is_auto_constant_v = is_auto_constant<T>::value;

template<typename...>
class tuple;

template<typename...>
class typelist;

template<auto...>
class constlist;

template<std::size_t... Is>
using indexlist = constlist<Is...>;

template<typename T>
struct typebox {
  using type = T;

  constexpr auto operator->() const noexcept -> const type * { return nullptr; }
};

} // namespace upd

template<typename... Ts>
struct std::tuple_size<upd::tuple<Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

template<std::size_t I, typename... Ts>
struct std::tuple_element<I, upd::tuple<Ts...>> {
  using type = upd::detail::variadic::at_t<std::tuple<Ts...>, I>;
};

template<typename... Ts>
struct std::tuple_size<upd::typelist<Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

template<std::size_t I, typename... Ts>
struct std::tuple_element<I, upd::typelist<Ts...>> {
  using type = upd::detail::variadic::at_t<std::tuple<Ts...>, I>;
};

template<auto... Vs>
struct std::tuple_size<upd::constlist<Vs...>> {
  constexpr static auto value = sizeof...(Vs);
};

template<std::size_t I, auto... Vs>
struct std::tuple_element<I, upd::constlist<Vs...>> {
  using type = upd::detail::variadic::at_t<std::tuple<upd::auto_constant<Vs>...>, I>;
};

namespace upd::detail {

template<typename F, std::size_t... Is>
[[nodiscard]] constexpr auto apply_on_index_sequence(F &&f, std::index_sequence<Is...>) -> decltype(auto) {
  return UPD_FWD(f)(std::integral_constant<std::size_t, Is>{}...);
}

template<std::size_t I, typename T>
struct leaf {
  [[nodiscard]] constexpr auto at(auto_constant<I>) &noexcept -> T & { return value; }

  [[nodiscard]] constexpr auto at(auto_constant<I>) const &noexcept -> const T & { return value; }

  [[nodiscard]] constexpr auto at(auto_constant<I>) &&noexcept -> T && { return std::move(value); }

  [[nodiscard]] constexpr auto at(auto_constant<I>) const &&noexcept -> const T && { return std::move(value); }

  [[nodiscard]] constexpr static auto typebox_at(auto_constant<I>) noexcept -> typebox<T>;

  T value;
};

template<typename, typename...>
struct leaves;

template<std::size_t... Is, typename... Ts>
struct leaves<std::index_sequence<Is...>, Ts...> : leaf<Is, Ts>... {
  using leaf<Is, Ts>::at...;
  using leaf<Is, Ts>::typebox_at...;

  template<std::size_t I>
  using raw_type = typename decltype(typebox_at(auto_constant<I>{}))::type;

  constexpr static auto size = sizeof...(Ts);

  explicit constexpr leaves() = default;

  explicit constexpr leaves(Ts... xs) : leaf<Is, Ts>{UPD_FWD(xs)}... {}

  [[nodiscard]] constexpr auto at(...) const noexcept -> variadic::not_found_t { return variadic::not_found; }
};

} // namespace upd::detail

namespace upd {

template<std::size_t N>
constexpr auto sequence = detail::apply_on_index_sequence([](auto... is) { return indexlist<is.value...>{}; },
                                                          std::make_index_sequence<N>{});

template<std::size_t, typename Tuple>
[[nodiscard]] constexpr auto get(Tuple &&) noexcept -> auto &&;

template<typename... Ts>
[[nodiscard]] constexpr auto fittest_tuple_like(Ts &&...);

template<typename Tuple, typename F>
[[nodiscard]] constexpr auto apply(Tuple &&, F &&);

template<typename Tuple, typename F>
constexpr void for_each(Tuple &&, F &&);

template<typename Derived>
class tuple_implementation {
  constexpr static auto normalize = [](auto &&...xs) noexcept { return fittest_tuple_like(UPD_FWD(xs)...); };

  [[nodiscard]] constexpr auto derived() noexcept -> Derived & { return reinterpret_cast<Derived &>(*this); }

  [[nodiscard]] constexpr auto derived() const noexcept -> const Derived & {
    return reinterpret_cast<const Derived &>(*this);
  }

public:
  template<typename I>
  [[nodiscard]] constexpr auto operator[](I i) &noexcept -> auto & {
    return get<i>(derived());
  }

  template<typename I>
  [[nodiscard]] constexpr auto operator[](I i) const &noexcept -> const auto & {
    return get<i>(derived());
  }

  template<typename I>
  [[nodiscard]] constexpr auto operator[](I i) &&noexcept -> auto && {
    return get<i>(std::move(derived()));
  }

  template<typename I>
  [[nodiscard]] constexpr auto operator[](I i) const &&noexcept -> const auto && {
    return get<i>(std::move(derived()));
  }

  template<typename I>
  [[nodiscard]] constexpr auto at(I i) &noexcept -> auto & {
    return get<i>(derived());
  }

  template<typename I>
  [[nodiscard]] constexpr auto at(I i) const &noexcept -> const auto & {
    return get<i>(derived());
  }

  template<typename I>
  [[nodiscard]] constexpr auto at(I i) &&noexcept -> auto && {
    return get<I>(std::move(derived()));
  }

  template<typename I>
  [[nodiscard]] constexpr auto at(I i) const &&noexcept -> const auto && {
    return get<I>(std::move(derived()));
  }

  [[nodiscard]] constexpr auto flatten() const {
    struct subelement_index {
      std::size_t sub_index;
      std::size_t index;
    };

    auto get_sub_size = [](auto sub) { return sub.size(); };
    auto retval_size = derived().transform_const(get_sub_size).fold_const(auto_constant<0>{}, plus);

    auto make_shape = [&](auto... xs) {
      auto shape = std::array<subelement_index, retval_size>{};
      auto cursor = shape.begin();
      auto sub_index = std::size_t{0};

      auto add_sub_shape = [&](const auto &sub) {
        for (auto i = std::size_t{0}; i < sub.size(); ++i, ++cursor) {
          *cursor = subelement_index{sub_index, i};
        }

        ++sub_index;
      };

      (add_sub_shape(xs), ...);

      return shape;
    };

    auto shape = derived().apply_const(make_shape, encapsulate);
    auto shape_seq = sequence<shape().size()>;
    auto get_subelement = [&](auto i) -> auto && {
      constexpr auto subelement_index = shape()[i];
      auto sub_index = auto_constant<subelement_index.sub_index>{};
      auto index = auto_constant<subelement_index.index>{};

      return UPD_FWD(derived()).at(sub_index).at(index);
    };

    return shape_seq.transform(get_subelement);
  }

  template<typename Init, typename BinaryOp>
  [[nodiscard]] constexpr auto fold_const(Init init, BinaryOp op) const noexcept {
    auto impl = [](auto init, auto op, auto... xs) {
      auto acc = init.value;
      ((void)(acc = op(acc, xs)), ...);

      return acc;
    };

    auto invoke_impl_const = [&](auto... xs) {
      constexpr auto result = impl(init, op, xs...);
      return auto_constant<result>{};
    };

    return derived().apply(invoke_impl_const);
  }

  template<template<typename> typename TT>
  [[nodiscard]] constexpr static auto metatransform() noexcept {
    auto apply_and_box_type = [](auto i) {
      using raw = typename Derived::template raw_type<i>;
      using result = TT<raw>;

      return typebox<result>{};
    };

    return sequence<size()>.transform(apply_and_box_type);
  }

  [[nodiscard]] constexpr auto square() {
    auto seq = sequence<size()>;
    auto pair_up = [&](auto i) { return seq.transform([&](auto j) { return indexlist<i, j>{}; }); };

    auto squared_seq = seq.transform(pair_up).flatten();

    auto make_pair = [&](auto ipair) {
      auto [i, j] = ipair;
      return std::pair{at(i), at(j)};
    };

    return squared_seq.transform(make_pair);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f) & {
    return transform_and_apply(derived(), UPD_FWD(f), normalize);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f) const & {
    return transform_and_apply(derived(), UPD_FWD(f), normalize);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f) && {
    return transform_and_apply(std::move(derived()), UPD_FWD(f), normalize);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f) const && {
    return transform_and_apply(std::move(derived()), UPD_FWD(f), normalize);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform_const(F f) const noexcept {
    auto apply_const = [&](auto x) { return auto_constant<f(x)>{}; };

    return derived().transform(apply_const);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform_type(F &&) const noexcept {
    auto apply_and_box_type = [](auto i) {
      using raw = typename Derived::template raw_type<i>;
      using result = std::invoke_result_t<F, raw>;

      return typebox<result>{};
    };

    return sequence<size()>.transform(apply_and_box_type);
  }

  [[nodiscard]] constexpr static auto type_only() noexcept {
    auto apply_and_box_type = [](auto i) {
      using raw = typename Derived::template raw_type<i>;
      return typebox<raw>{};
    };

    return sequence<size()>.transform(apply_and_box_type);
  }

  template<typename F>
  [[nodiscard]] constexpr auto apply(F &&f) & -> decltype(auto) {
    return upd::apply(derived(), UPD_FWD(f));
  }

  template<typename F>
  [[nodiscard]] constexpr auto apply(F &&f) const & -> decltype(auto) {
    return upd::apply(derived(), UPD_FWD(f));
  }

  template<typename F>
  [[nodiscard]] constexpr auto apply(F &&f) && -> decltype(auto) {
    return upd::apply(std::move(derived()), UPD_FWD(f));
  }

  template<typename F>
  [[nodiscard]] constexpr auto apply(F &&f) const && -> decltype(auto) {
    return upd::apply(std::move(derived()), UPD_FWD(f));
  }

  template<typename F>
  [[nodiscard]] constexpr auto apply_const(F f, encapsulate_t) const noexcept {
    auto impl = [&](auto... xs) {
      constexpr auto result = f(xs...);
      return [&] { return result; };
    };

    return derived().apply(impl);
  }

  template<typename F>
  constexpr void for_each(F &&f) & {
    upd::for_each(derived(), UPD_FWD(f));
  }

  template<typename F>
  constexpr void for_each(F &&f) const & {
    upd::for_each(derived(), UPD_FWD(f));
  }

  template<typename F>
  constexpr void for_each(F &&f) && {
    upd::for_each(std::move(derived()), UPD_FWD(f));
  }

  template<typename F>
  constexpr void for_each(F &&f) const && {
    upd::for_each(std::move(derived()), UPD_FWD(f));
  }

  [[nodiscard]] constexpr static auto size() noexcept -> std::size_t { return std::tuple_size_v<Derived>; }
};

template<typename... Ts>
class tuple : public tuple_implementation<tuple<Ts...>> {
  template<std::size_t, typename Tuple>
  friend constexpr auto get(Tuple &&) noexcept -> auto &&;

  using leaves = detail::leaves<std::index_sequence_for<Ts...>, Ts...>;

public:
  template<std::size_t I>
  using raw_type = typename leaves::template raw_type<I>;

  template<typename Tuple>
  [[nodiscard]] constexpr static auto normalize(Tuple &&tuple) {
    return fittest_tuple_like(UPD_FWD(tuple));
  }

  explicit constexpr tuple(Ts... xs) noexcept : m_leaves{std::move(xs)...} {}

private:
  leaves m_leaves;
};

template<typename... Ts>
class typelist : public tuple_implementation<typelist<Ts...>> {
  template<std::size_t, typename Tuple>
  friend constexpr auto get(Tuple &&) noexcept -> auto &&;

  using leaves = detail::leaves<std::index_sequence_for<Ts...>, typebox<Ts>...>;

public:
  template<std::size_t I>
  using raw_type = typename leaves::template raw_type<I>::type;

  template<typename Tuple>
  [[nodiscard]] constexpr static auto normalize(Tuple &&tuple) {
    return fittest_tuple_like(UPD_FWD(tuple));
  }

  constexpr typelist() noexcept = default;

private:
  leaves m_leaves;
};

template<auto... Vs>
class constlist : public tuple_implementation<constlist<Vs...>> {
  template<std::size_t, typename Tuple>
  friend constexpr auto get(Tuple &&) noexcept -> auto &&;

  using leaves = detail::leaves<std::make_index_sequence<sizeof...(Vs)>, auto_constant<Vs>...>;

public:
  template<std::size_t I>
  using raw_type = typename leaves::template raw_type<I>;

  template<typename Tuple>
  [[nodiscard]] constexpr static auto normalize(Tuple &&tuple) {
    return fittest_tuple_like(UPD_FWD(tuple));
  }

  constexpr constlist() noexcept = default;

private:
  leaves m_leaves;
};

template<typename... Ts>
[[nodiscard]] constexpr auto fittest_tuple_like(Ts &&...xs) {
  if constexpr ((detail::is_instance_of_v<std::decay_t<Ts>, typebox> && ...)) {
    return typelist<typename std::decay_t<Ts>::type...>{};
  } else if constexpr ((is_auto_constant_v<Ts> && ...)) {
    return constlist<xs.value...>{};
  } else {
    return tuple{UPD_FWD(xs)...};
  }
}

template<typename... Ts, typename... Us>
[[nodiscard]] constexpr auto operator+(tuple<Ts...> lhs, tuple<Us...> rhs) noexcept -> tuple<Ts..., Us...> {
  auto impl = [&](auto i) {
    auto lhs_size = auto_constant<sizeof...(Ts)>{};

    if constexpr (i < sizeof...(Ts)) {
      return std::move(lhs[i]);
    } else {
      return std::move(rhs[i - lhs_size]);
    }
  };

  return sequence<sizeof...(Ts) + sizeof...(Us)>.transform(impl);
}

template<std::size_t I, typename Tuple>
[[nodiscard]] constexpr auto get(Tuple &&tuple) noexcept -> auto && {
  auto &&retval = UPD_FWD(tuple).m_leaves.at(auto_constant<I>{});

  using retval_type = decltype(retval);
  static_assert(!std::is_same_v<retval_type, detail::variadic::not_found_t &&>, "`I` is not a valid index for `tuple`");

  return retval;
}

template<typename Tuple, typename F, typename Aggregator>
[[nodiscard]] constexpr auto transform_and_apply(Tuple &&t, F &&f, Aggregator &&agg) {
  constexpr auto size = std::tuple_size_v<std::remove_reference_t<Tuple>>;
  constexpr auto seq = std::make_index_sequence<size>{};

  auto invoke_f_once = [&](auto i) -> decltype(auto) { return f(get<i>(UPD_FWD(t))); };

  auto invoke_f_on_each = [&](auto... is) -> decltype(auto) { return UPD_FWD(agg)(invoke_f_once(is)...); };

  return detail::apply_on_index_sequence(invoke_f_on_each, seq);
}

template<typename Tuple, typename F>
[[nodiscard]] constexpr auto apply(Tuple &&t, F &&f) {
  constexpr auto size = std::tuple_size_v<std::remove_reference_t<Tuple>>;
  constexpr auto seq = std::make_index_sequence<size>{};

  auto apply_f = [&](auto... is) -> decltype(auto) { return UPD_FWD(f)(get<is>(UPD_FWD(t))...); };

  return detail::apply_on_index_sequence(apply_f, seq);
}

template<typename Tuple, typename F>
constexpr void for_each(Tuple &&t, F &&f) {
  auto impl = [&](auto &&...xs) { ((void)f(UPD_FWD(xs)), ...); };

  apply(UPD_FWD(t), impl);
}

} // namespace upd
