#pragma once

#include <optional>
#include <numeric>
#include <ranges>
#include <functional>
#include "detail/variadic/at.hpp"
#include "detail/variadic/leaf.hpp"
#include "detail/variadic/clean.hpp"
#include "detail/is_instance_of.hpp"
#include "upd.hpp"

namespace upd {

template<typename T, template<typename...> typename TT>
concept instance_of = detail::is_instance_of_v<T, TT>;

template<typename T> requires std::is_reference_v<T>
class ref {
public:
  using qualified_type = T;

  constexpr explicit ref(T target) noexcept(release): m_target{UPD_FWD(target)} {}

  [[nodiscard]] constexpr operator T() noexcept(release) {
    return UPD_FWD(m_target);
  }
  
private:
  T m_target;
};

template<typename T>
explicit ref(T &&) -> ref<T &&>;

template<typename>
class tuple_implementation;

template<typename T, template<typename...> typename TT>
concept cvref_instance_of = detail::is_instance_of_v<std::remove_cvref_t<T>, TT>;

template<typename T, typename BinaryOp>
class accumulable_t {
  template<typename U, typename _T, std::invocable<U, _T> _BinaryOp>
  friend constexpr auto operator,(U &&, accumulable_t<_T, _BinaryOp> &&) -> decltype(auto);

  template<typename _T, typename _BinaryOp>
  friend constexpr auto accumulable(_T &&, _BinaryOp &&) noexcept -> accumulable_t<_T &&, _BinaryOp &&>;

  constexpr accumulable_t(T value, BinaryOp op): m_value{UPD_FWD(value)}, m_op{UPD_FWD(op)}  {}

  T m_value;
  BinaryOp m_op;
};

template<typename U, typename T, std::invocable<U, T> BinaryOp>
[[nodiscard]] constexpr auto operator,(U &&x, accumulable_t<T, BinaryOp> &&acc) -> decltype(auto) {
  return std::invoke(UPD_FWD(acc.m_op), UPD_FWD(x), UPD_FWD(acc.m_value));
}

template<typename T, typename BinaryOp>
[[nodiscard]] constexpr auto accumulable(T &&x, BinaryOp &&op) noexcept -> accumulable_t<T &&, BinaryOp &&> {
  return accumulable_t<T &&, BinaryOp &&>{UPD_FWD(x), UPD_FWD(op)};
}

struct filter_void_t {};

constexpr auto filter_void = filter_void_t{};

struct encapsulate_t {};

constexpr auto encapsulate = encapsulate_t{};

constexpr inline auto equal_to = [](auto &&lhs, auto &&rhs) {
  return UPD_FWD(lhs) == UPD_FWD(rhs);
};

constexpr inline auto plus = [](auto &&lhs, auto &&rhs) {
  return UPD_FWD(lhs) + UPD_FWD(rhs);
};

constexpr inline auto invoke = [](auto &&f, auto && ...args) {
  return UPD_FWD(f)(UPD_FWD(args)...);
};

struct preserve_value_category_t {};

constexpr auto preserve_value_category = preserve_value_category_t{};

struct unpack_t {};

constexpr auto unpack = unpack_t{};

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

template<auto Expr>
constexpr auto expr = auto_constant<Expr>{};

template<typename T>
concept auto_constant_instance = requires(T x) {
  { auto_constant{x} } -> std::same_as<T>;
};

template<auto Lhs, auto Rhs>
requires std::equality_comparable_with<decltype(Lhs), decltype(Rhs)>
[[nodiscard]] constexpr auto operator==(auto_constant<Lhs>, auto_constant<Rhs>) noexcept(release) {
  return expr<Lhs == Rhs>;
}

template<auto_constant_instance Lhs, typename Rhs>
requires requires { Lhs::value == std::declval<Rhs>(); }
[[nodiscard]] consteval auto operator==(Lhs lhs, Rhs rhs) -> bool {
  return lhs.value == rhs;
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
  using unqualified_type = std::remove_cvref_t<T>;

  constexpr auto operator->() const noexcept -> const unqualified_type * { return nullptr; }
};

} // namespace upd

namespace upd {

template<std::size_t N>
constexpr auto sequence = []<std::size_t... Is>(std::index_sequence<Is...>) {
  return indexlist<Is...>{};
}(std::make_index_sequence<N>{});

using false_type = auto_constant<false>;
using true_type = auto_constant<true>;

template<typename, typename = void>
struct has_type_member : false_type {};

template<typename T>
struct has_type_member<T, std::void_t<typename T::type>> : true_type {};

template<typename T>
constexpr auto has_type_member_v = has_type_member<T>::value;

template<typename, typename = void>
struct has_value_member : false_type {};

template<typename T>
struct has_value_member<T, decltype((void) T::value)> : true_type {};

template<typename T>
constexpr auto has_value_member_v = has_value_member<T>::value;

template<typename Value, auto Default>
constexpr auto constant_value_or = []() {
  if constexpr (has_value_member_v<Value>) {
    return Value::value;
  } else {
    return Default;
  }
}();

} // namespace upd

namespace upd {

template<std::size_t, typename Tuple>
[[nodiscard]] constexpr auto get(Tuple &&) noexcept -> auto &&;

} // namespace upd

namespace upd {

template<typename T>
concept tuple_like = requires(std::remove_reference_t<T> x) {
  std::tuple_size<decltype(x)>::value;
  { std::tuple_size_v<decltype(x)> } -> std::convertible_to<std::size_t>;
}
&& []<std::size_t... Is>(std::index_sequence<Is...>) {
  auto has_tuple_element = [](auto i) {
    return requires(std::remove_reference_t<T> x) {
      typename std::tuple_element_t<i, decltype(x)>;
    };
  };
  return (has_tuple_element(auto_constant<Is>{}) && ...);
} (std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{})
&&  requires(std::remove_reference_t<T> x) {
  { get<0>(x) } -> std::same_as<std::tuple_element_t<0, decltype(x)> &>;
}
&& []<std::size_t... Is>(std::index_sequence<Is...>) {
  auto is_nth_gettable = [](auto i) {
    return requires(std::remove_reference_t<T> x) {
      { get<i>(x) } -> std::same_as<std::tuple_element_t<i, decltype(x)> &>;
    };
  };
  return (is_nth_gettable(auto_constant<Is>{}) && ...);
} (std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{})
&& []<std::size_t... Is>(std::index_sequence<Is...>) {
  auto is_nth_moved_gettable = [](auto i) {
    return requires(std::remove_reference_t<T> x) {
      { get<i>(std::move(x)) } -> std::same_as<std::tuple_element_t<i, decltype(x)> &&>;
    };
  };
  return (is_nth_moved_gettable(auto_constant<Is>{}) && ...);
} (std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});

template<typename T, template<typename> typename Base>
concept implementer = std::derived_from<T, Base<T>>;

template<tuple_like Tuple, typename F, typename Aggregator>
constexpr auto transform_and_apply(Tuple &&t, F &&f, Aggregator &&agg) -> decltype(auto) {
  auto impl = [&](auto &&... xs) -> decltype(auto) {
    return std::invoke(
      UPD_FWD(agg),
      std::invoke(f, UPD_FWD(xs))...
    );
  };

  return apply(UPD_FWD(t), impl);
}

template<tuple_like Tuple, typename F>
constexpr auto apply(Tuple &&t, F &&f) -> decltype(auto) {
  constexpr auto size = std::tuple_size_v<std::remove_reference_t<Tuple>>;
  constexpr auto seq = std::make_index_sequence<size>{};

  return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
    return UPD_FWD(f)(get<Is>(UPD_FWD(t))...);
  }(seq);
}

template<typename F>
[[nodiscard]] constexpr auto operator|(unpack_t, F &&f) {
  auto impl = [&](auto &&t) -> decltype(auto) {
    return apply(UPD_FWD(t), UPD_FWD(f));
  };

  return impl;
}

} // namespace upd

template<upd::implementer<upd::tuple_implementation> T>
struct std::tuple_size<T> {
  constexpr static auto value = T::size();
};

template<std::size_t I, upd::implementer<upd::tuple_implementation> T>
struct std::tuple_element<I, T> {
  using type = typename T::template raw_type<I>;
};

namespace upd::detail {

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

  constexpr leaves() = default;

  template<typename ...Us>
  explicit constexpr leaves(Us &&... xs) : leaf<Is, Ts>{UPD_FWD(xs)}... {}

  [[nodiscard]] constexpr auto at(...) const noexcept -> variadic::not_found_t { return variadic::not_found; }
};

template<typename... Ts>
leaves(Ts...) -> leaves<Ts...>;

} // namespace upd::detail

namespace upd {

template<std::size_t, typename Tuple>
[[nodiscard]] constexpr auto get(Tuple &&) noexcept -> auto &&;

template<typename... Ts>
[[nodiscard]] constexpr auto fittest_tuple_like(Ts &&...);

template<typename Tuple, typename F>
constexpr void for_each(Tuple &&, F &&);

template<typename Tuple, typename F>
[[nodiscard]] constexpr auto transform(Tuple &&, F &&);

template<typename Tuple, typename F>
[[nodiscard]] constexpr auto transform(Tuple &&, F &&, preserve_value_category_t);

template<typename Tuple, typename F>
[[nodiscard]] constexpr auto transform(Tuple &&, F &&, filter_void_t);

template<typename Derived>
class tuple_implementation {
  constexpr static auto normalize = [](auto &&...xs) noexcept { return fittest_tuple_like(UPD_FWD(xs)...); };

  [[nodiscard]] constexpr auto derived() noexcept -> Derived & { return static_cast<Derived &>(*this); }

  [[nodiscard]] constexpr auto derived() const noexcept -> const Derived & {
    return static_cast<const Derived &>(*this);
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
    return get<i>(std::move(derived()));
  }

  template<typename I>
  [[nodiscard]] constexpr auto at(I i) const &&noexcept -> const auto && {
    return get<i>(std::move(derived()));
  }

  template<typename UnaryPred>
  [[nodiscard]] constexpr auto all_of(UnaryPred &&p) const -> bool {
    auto check = [&](auto truth, const auto &x) { return truth && p(x); };
    return fold_left(true, check);
  }

  [[nodiscard]] constexpr auto clean() const {
    using namespace std::ranges::views;

    constexpr auto truth_table = sequence<Derived::size()>
      .apply([](auto... is) {
          return std::array {
            !std::same_as<
              typename Derived::template raw_type<is>,
              detail::variadic::marked_for_cleaning_t
            >...
          };
      });

    constexpr auto kept_index_count = std::accumulate(truth_table.begin(), truth_table.end(), 0);
    constexpr auto kept_indices = [&] {
      auto kept_indices = std::array<std::size_t, kept_index_count> {};
      auto i = std::size_t{0};
      for (auto j : iota(std::size_t{0}, truth_table.size())) {
        if (truth_table.at(j)) {
          kept_indices.at(i) = j;
          ++i;
        }
      }
      UPD_ASSERT(i == kept_indices.size());

      return kept_indices;
    }();

    return sequence<kept_indices.size()>
      .transform([&](auto i) { return auto_constant<kept_indices.at(i)>{}; })
      .transform([&](auto i) -> auto&& { return at(i); });
  }

  template<typename Pred>
  [[nodiscard]] constexpr auto filter(Pred &&p) const {
    auto impl = [&](auto &&x) -> decltype(auto) {
      auto truth = std::invoke(UPD_FWD(p), std::as_const(x));
      if constexpr (truth) {
        return UPD_FWD(x);
      } else {
        return detail::variadic::marked_for_cleaning_t{};
      }
    };

    return transform(impl).clean();
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

  template<typename UnaryPred>
  [[nodiscard]] constexpr auto find_const(UnaryPred p) const noexcept(release) {
    auto elligibles = derived()
      .enumerate()
      .filter(unpack | [p](auto, auto type) { return std::invoke(p, type); })
      .transform(unpack | [](auto i, const auto &) { return i; });

    if constexpr (elligibles.size() > 0) {
      return elligibles[auto_constant<0>{}];
    } else {
      return auto_constant<Derived::size()>{};
    }
  }

  [[nodiscard]] constexpr auto enumerate() const noexcept(release) {
    return sequence<Derived::size()>
      .transform([&](auto i) { return tuple{i, ref(UPD_FWD(at(i)))}; });
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

  template<typename Init, typename BinaryOp>
  [[nodiscard]] constexpr auto fold_left(Init &&init, BinaryOp &&op) const noexcept {
    auto impl = [&](auto &&... xs) {
      return (UPD_FWD(init), ..., accumulable(op, UPD_FWD(xs)));
    };

    return derived().apply(impl);
  }

  template<typename Init, typename BinaryOp>
  [[nodiscard]] constexpr auto fold_right(Init &&init, BinaryOp &&op) const noexcept {
    auto flipped_op = [&](auto &&lhs, auto &&rhs) { return op(UPD_FWD(rhs), UPD_FWD(lhs)); };
    return derived().reverse().fold_left(UPD_FWD(init), flipped_op);
  }

  template<template<typename> typename TT>
  [[nodiscard]] constexpr static auto metatransform() noexcept {
    auto apply_and_box_type = [](auto i) {
      using raw = typename Derived::template raw_type<i>;
      using result = TT<raw>;

      return typebox<result>{};
    };

    return sequence<Derived::size()>.transform(apply_and_box_type);
  }

  [[nodiscard]] constexpr auto reverse() const {
    constexpr auto rindices = []() {
      auto rindices = std::array<std::size_t, Derived::size()>{};
      auto first = rindices.rbegin();
      auto last = rindices.rend();
      auto i = std::size_t{0};
      for (auto &ri : detail::range{first, last}) {
        ri = i++;
      }

      return rindices;
    }();

    auto get_element = [&](auto i) -> auto && {
      return get<rindices[i]>(derived());
    };
    return sequence<Derived::size()>.transform(get_element, preserve_value_category);
  }

  [[nodiscard]] constexpr auto square() {
    auto seq = sequence<Derived::size()>;
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
  [[nodiscard]] constexpr auto transform(F &&f, preserve_value_category_t) & {
    return upd::transform(derived(), UPD_FWD(f), preserve_value_category);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f, preserve_value_category_t) const & {
    return upd::transform(derived(), UPD_FWD(f), preserve_value_category);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f, preserve_value_category_t) && {
    return upd::transform(std::move(derived()), UPD_FWD(f), preserve_value_category);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f, preserve_value_category_t) const && {
    return upd::transform(std::move(derived()), UPD_FWD(f), preserve_value_category);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f, filter_void_t) & {
    return upd::transform(derived(), UPD_FWD(f), filter_void);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f, filter_void_t) const & {
    return upd::transform(derived(), UPD_FWD(f), filter_void);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f, filter_void_t) && {
    return upd::transform(std::move(derived()), UPD_FWD(f), filter_void);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform(F &&f, filter_void_t) const && {
    return upd::transform(std::move(derived()), UPD_FWD(f), filter_void);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform_const(F f) const noexcept {
    auto apply_const = [f](auto x) { return expr<std::invoke(f, x)>; };

    return derived().transform(apply_const);
  }

  template<typename F>
  [[nodiscard]] constexpr auto transform_type(F &&) const noexcept {
    auto apply_and_box_type = [](auto i) {
      using raw = typename Derived::template raw_type<i>;
      using result = std::invoke_result_t<F, raw>;

      return typebox<result>{};
    };

    return sequence<Derived::size()>.transform(apply_and_box_type);
  }

  [[nodiscard]] constexpr static auto type_only() noexcept {
    auto apply_and_box_type = [](auto i) {
      using raw = typename Derived::template raw_type<i>;
      return typebox<raw>{};
    };

    return sequence<Derived::size()>.transform(apply_and_box_type);
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
      return [result] { return result; };
    };

    return derived().apply(impl);
  }

  template<template<typename...> typename TT, typename... Args>
  [[nodiscard]] constexpr auto apply_template(Args &&... args) const noexcept {
    auto instantiate_and_construct = [&](auto... types) {
      return TT<typename decltype(types)::type...>{UPD_FWD(args)...};
    };

    return derived().type_only().apply(instantiate_and_construct);
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

  [[nodiscard]] constexpr static auto size() noexcept -> std::size_t {
    return sizeof...(Ts);
  }

  template<typename ...Us>
  constexpr explicit tuple(Us &&... xs) noexcept(release) : m_leaves{UPD_FWD(xs)...} {}

  template<typename ...Us>
  constexpr explicit tuple(std::in_place_t, Us &&... xs) noexcept(release) : m_leaves{UPD_FWD(xs)...} {}

private:
  leaves m_leaves;
};

template<typename ...Ts>
explicit tuple(Ts...) -> tuple<
  typename decltype([] {
      if constexpr (cvref_instance_of<Ts, ref>) {
        return typebox<typename Ts::qualified_type>{};
      } else {
        return typebox<Ts>{};
      }
  } ())::type...
>;

template<typename ...Ts>
explicit tuple(std::in_place_t, Ts...) -> tuple<
  typename decltype([] {
      if constexpr (cvref_instance_of<Ts, ref>) {
        return typebox<typename Ts::qualified_type>{};
      } else {
        return typebox<Ts>{};
      }
  } ())::type...
>;

template<typename... Ts>
class typelist : public tuple_implementation<typelist<Ts...>> {
  template<std::size_t, typename Tuple>
  friend constexpr auto get(Tuple &&) noexcept -> auto &&;

  using leaves = detail::leaves<std::index_sequence_for<Ts...>, typebox<Ts>...>;

public:
  template<std::size_t I>
  using raw_type = typename leaves::template raw_type<I>;

  template<typename Tuple>
  [[nodiscard]] constexpr static auto normalize(Tuple &&tuple) {
    return fittest_tuple_like(UPD_FWD(tuple));
  }

  [[nodiscard]] constexpr static auto size() noexcept -> std::size_t {
    return sizeof...(Ts);
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

  [[nodiscard]] constexpr static auto size() noexcept -> std::size_t {
    return sizeof...(Vs);
  }

  constexpr constlist() noexcept = default;

private:
  leaves m_leaves;
};

template<tuple_like auto TupleLike>
constexpr auto as_constlist = sequence<std::tuple_size_v<decltype(TupleLike)>>
  .transform_const([](auto i) { return std::get<i>(TupleLike); });

template<typename... Ts>
[[nodiscard]] constexpr auto fittest_tuple_like(Ts &&...xs) {
  if constexpr ((detail::is_instance_of_v<std::decay_t<Ts>, typebox> && ...)) {
    return typelist<typename std::decay_t<Ts>::type...>{};
  } else if constexpr ((is_auto_constant_v<Ts> && ...)) {
    return constlist<xs.value...>{};
  } else {
    return tuple{std::in_place, UPD_FWD(xs)...};
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

  return UPD_FWD(retval);
}

template<typename Tuple, typename F>
constexpr void for_each(Tuple &&t, F &&f) {
  auto impl = [&](auto &&...xs) { ((void)f(UPD_FWD(xs)), ...); };

  upd::apply(UPD_FWD(t), impl);
}

template<typename Tuple, typename F>
[[nodiscard]] constexpr auto transform(Tuple &&t, F &&f, preserve_value_category_t) {
  auto impl = [&](auto &&...xs) {
    return tuple<std::invoke_result_t<F, decltype(xs)>...>{f(UPD_FWD(xs))...};
  };

  return upd::apply(UPD_FWD(t), impl);
}

template<typename Tuple, typename F>
[[nodiscard]] constexpr auto transform(Tuple &&t, F &&f, filter_void_t) {
  auto invoke_f = [&](auto &&x) -> decltype(auto) {
    using type = decltype(x);
    using invoke_result = std::invoke_result_t<F, type>;

    if constexpr (std::is_void_v<invoke_result>) {
      std::invoke(f, UPD_FWD(x));
      return detail::variadic::marked_for_cleaning_t{};
    } else {
      return std::invoke(f, UPD_FWD(x));
    }
  };

  auto impl = [&](auto &&...xs) {
    return tuple{invoke_f(UPD_FWD(xs))...};
  };

  return upd::apply(UPD_FWD(t), impl).clean();
}

template<tuple_like... Tuples>
[[nodiscard]] constexpr auto zip(Tuples &&... ts) {
  constexpr auto min_size = std::min(ts.size()...);

  return sequence<min_size>
    .transform([&](auto i) {
        return tuple{ref{UPD_FWD(ts).at(i)}...};
    });
}

} // namespace upd
