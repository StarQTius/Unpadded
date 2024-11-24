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
#include "ref.hpp"
#include "template_traits.hpp"
#include "functional.hpp"
#include "constexpr.hpp"
#include "type_traits.hpp"

#define UPD_CONSTEXPR_ASSERT(...) \
  (std::is_constant_evaluated() && (__VA_ARGS__) ? (void) 0 : throw)

#define UPD_THIS_DEDUCTION_REQUIRES_WORKAROUND(...) \
  static_assert(__VA_ARGS__)

#define UPD_VARIADIC_CONSTRAINT_WORKAROUND(...) \
  ((__VA_ARGS__) && ...)

#define UPD_INVOKE(INVOCABLE, ...) ((INVOCABLE)(__VA_ARGS__))

namespace upd::detail {

template<std::size_t I, typename T>
struct leaf{
  [[nodiscard]] constexpr auto at(auto_constant<I>) &noexcept -> T & { return value; }

  [[nodiscard]] constexpr auto at(auto_constant<I>) const &noexcept -> const T & { return value; }

  [[nodiscard]] constexpr auto at(auto_constant<I>) &&noexcept -> T && { return UPD_FWD(value); }

  [[nodiscard]] constexpr auto at(auto_constant<I>) const &&noexcept -> const T && { return UPD_FWD(value); }

  [[nodiscard]] constexpr static auto typebox_at(auto_constant<I>) noexcept -> typebox<T>;

  [[nodiscard]] constexpr static auto has_type(typebox<T>) noexcept(release) -> bool {
    return true;
  }

  T value;
};

template<typename, typename...>
struct leaves;

template<std::size_t... Is, typename... Ts>
struct leaves<std::index_sequence<Is...>, Ts...> : leaf<Is, Ts>... {
  using leaf<Is, Ts>::at...;
  using leaf<Is, Ts>::typebox_at...;
  using leaf<Is, Ts>::has_type...;

  [[nodiscard]] constexpr auto typebox_at(...) const noexcept -> variadic::not_found_t { return variadic::not_found; }

  [[nodiscard]] constexpr static auto has_type(...) noexcept(release) -> bool { return false; }

  template<std::size_t I>
  using raw_type = typename decltype(typebox_at(auto_constant<I>{}))::type;

  constexpr static auto size = sizeof...(Ts);

  constexpr leaves() = default;

  template<typename ...Us>
  explicit constexpr leaves(Us &&... xs) : leaf<Is, Ts>{UPD_FWD(xs)}... {}

  [[nodiscard]] constexpr auto at(...) const noexcept -> variadic::not_found_t { return variadic::not_found; }
};

template<typename... Ts>
explicit leaves(Ts...) -> leaves<Ts...>;

template<typename... Ts>
using lite_tuple = leaves<std::index_sequence_for<Ts...>, Ts...>;

} // namespace upd::detail

namespace upd {

template<typename Target, typename From>
struct transfert_reference {
  using type = Target;
};

template<typename Target, typename From>
struct transfert_reference<Target, From &> {
  using type = Target &;
};

template<typename Target, typename From>
struct transfert_reference<Target, From &&> {
  using type = Target &&;
};

template<typename Target, typename From>
using transfert_reference_t = typename transfert_reference<Target, From>::type;

constexpr struct unpack_t {} unpack;

template<typename F>
[[nodiscard]] constexpr auto to_metafunction(F f) {
  return [=]<metavalue... Metas>(Metas...) {
    return expr<UPD_INVOKE(f, Metas::value...)>;
  };
}

template<typename T, typename U>
struct reference_like {
  using type = std::remove_reference_t<T>;
};

template<typename T, typename U>
struct reference_like<T, U&> {
  using type = std::remove_reference_t<T> &;
};

template<typename T, typename U>
struct reference_like<T, U&&> {
  using type = std::remove_reference_t<T> &&;
};

template<typename T, typename U>
using reference_like_t = typename reference_like<T, U>::type;

template<typename T, typename BinaryOp>
class accumulable_t {
  template<typename _T, typename U, std::invocable<_T, U> _BinaryOp>
  friend constexpr auto operator,(accumulable_t<_T, _BinaryOp> &&, U &&) -> decltype(auto);

  template<typename U, typename _T, std::invocable<U, _T> _BinaryOp>
  friend constexpr auto operator,(U &&, accumulable_t<_T, _BinaryOp> &&) -> decltype(auto);

  template<typename _T, typename _BinaryOp>
  friend constexpr auto accumulable(_T &&, _BinaryOp &&) noexcept(release) -> accumulable_t<_T &&, _BinaryOp &&>;

  constexpr accumulable_t(T value, BinaryOp op): m_value{UPD_FWD(value)}, m_op{UPD_FWD(op)}  {}

  T m_value;
  BinaryOp m_op;
};

template<typename T, typename U, std::invocable<T, U> BinaryOp>
[[nodiscard]] constexpr auto operator,(accumulable_t<T, BinaryOp> &&acc, U &&x) -> decltype(auto) {
  return UPD_INVOKE(UPD_FWD(acc.m_op), UPD_FWD(acc.m_value), UPD_FWD(x));
}

template<typename U, typename T, std::invocable<U, T> BinaryOp>
[[nodiscard]] constexpr auto operator,(U &&x, accumulable_t<T, BinaryOp> &&acc) -> decltype(auto) {
  return UPD_INVOKE(UPD_FWD(acc.m_op), UPD_FWD(x), UPD_FWD(acc.m_value));
}

template<typename T, typename BinaryOp>
[[nodiscard]] constexpr auto accumulable(T &&x, BinaryOp &&op) noexcept(release) -> accumulable_t<T &&, BinaryOp &&> {
  return accumulable_t<T &&, BinaryOp &&>{UPD_FWD(x), UPD_FWD(op)};
}

template<typename...>
class tuple;

template<typename...>
class typelist;

template<auto...>
class constlist;

template<typename>
class tuple_implementation;

template<typename T>
concept tuple_like = requires(std::remove_reference_t<T> x) {
  std::tuple_size<decltype(x)>::value;
  { std::tuple_size_v<decltype(x)> } -> std::convertible_to<std::size_t>;
}
&& []<std::size_t... Is>(std::index_sequence<Is...>) {
  [[maybe_unused]] auto has_tuple_element = [](auto i) {
    return requires(std::remove_reference_t<T> x) {
      typename std::tuple_element_t<i, decltype(x)>;
    };
  };
  return (has_tuple_element(auto_constant<Is>{}) && ...);
} (std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{})
&&
[]<std::size_t... Is>(std::index_sequence<Is...>) {
  [[maybe_unused]] auto is_nth_gettable = []([[maybe_unused]] auto i) {
    return requires(T &&x) {
      { get<i>(UPD_FWD(x)) } -> std::same_as<
        transfert_reference_t<std::tuple_element_t<i, std::remove_reference_t<T>> &&, T &&>
      >;
    };
  };
  return (is_nth_gettable(auto_constant<Is>{}) && ...);
}(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});

template<std::size_t I, typename Tuple>
[[nodiscard]] constexpr auto get(Tuple &&t) noexcept(release) -> auto && {
  return UPD_FWD(t).template get<I, Tuple>();
}

template<std::size_t N>
constexpr auto sequence = []<std::size_t... Is>(std::index_sequence<Is...>) {
  return constlist<Is...>{};
}(std::make_index_sequence<N>{});

template<tuple_like Tuple>
constexpr auto sequence_for = sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>;

} // namespace upd

template<typename... Ts>
struct std::tuple_size<upd::tuple<Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

template<std::size_t I, typename... Ts>
struct std::tuple_element<I, upd::tuple<Ts...>> {
  using type = typename decltype(auto{upd::detail::lite_tuple<upd::typebox<Ts>...>{}.at(upd::expr<I>)})::type;
};

template<typename... Ts>
struct std::tuple_size<upd::typelist<Ts...>> {
  constexpr static auto value = sizeof...(Ts);
};

template<std::size_t I, typename... Ts>
struct std::tuple_element<I, upd::typelist<Ts...>> {
  using type = decltype(auto{upd::detail::lite_tuple<upd::typebox<Ts>...>{}.at(upd::expr<I>)});
};

template<auto... Values>
struct std::tuple_size<upd::constlist<Values...>> {
  constexpr static auto value = sizeof...(Values);
};

template<std::size_t I, auto... Values>
struct std::tuple_element<I, upd::constlist<Values...>> {
  using type = decltype(auto{upd::detail::lite_tuple<upd::auto_constant<Values>...>{}.at(upd::expr<I>)});
};

namespace upd {

template<typename T>
concept nested_tuple = tuple_like<T> && []<std::size_t... Is>(std::index_sequence<Is...>) {
  using noref_type [[maybe_unused]] = std::remove_reference_t<T>;
  return (tuple_like<std::tuple_element_t<Is, noref_type>> && ...);
}(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});

template<typename T>
concept typelist_like = tuple_like<T> && []<std::size_t... Is>(std::index_sequence<Is...>) {
  using noref_type [[maybe_unused]] = std::remove_reference_t<T>;
  return (metatype<std::tuple_element_t<Is, noref_type>> && ...);
}(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});

template<typename T>
concept constlist_like = tuple_like<T> && []<std::size_t... Is>(constlist<Is...>) {
  using noref_type [[maybe_unused]] = std::remove_reference_t<T>;
  return (metavalue<std::tuple_element_t<Is, T>> && ...);
}(sequence_for<T>);

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
concept named_tuple_like = tuple_like<T>
&& []<std::size_t... Is>(std::index_sequence<Is...>) {
  [[maybe_unused]] auto has_tuple_identifier = [](auto i) {
    return requires(std::remove_reference_t<T> x) {
      named_tuple_identifier<i, decltype(x)>::value;
    };
  };
  return (has_tuple_identifier(expr<Is>) && ...);
} (std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{})
&& []<std::size_t... Is>(std::index_sequence<Is...>) {
  [[maybe_unused]] auto has_tuple_identifier = [](auto i) {
    return requires(std::remove_reference_t<T> x) {
      typename named_tuple_element<i, decltype(x)>::type;
    };
  };
  return (has_tuple_identifier(expr<Is>) && ...);
} (std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{})
&& []<std::size_t... Is>(std::index_sequence<Is...>) {
  [[maybe_unused]] auto is_nth_id_gettable = []([[maybe_unused]] auto i) {
    using noref_type = std::remove_reference_t<T>;
    [[maybe_unused]] constexpr auto identifier = named_tuple_identifier_v<i, noref_type>;
    return requires(T &&x) {
      { get<named_tuple_identifier_v<i, noref_type>>(UPD_FWD(x)) } -> std::same_as<
        transfert_reference_t<named_tuple_element_t<i, noref_type> &&, T &&>
      >;
    };
  };
  return (is_nth_id_gettable(expr<Is>) && ...);
}(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<T>>>{});

template<typename T>
concept array_like = tuple_like<T>
&& std::tuple_size_v<T> > 0
&& []<std::size_t... Is>(constlist<Is...>) {
  using first_element_type = std::remove_cvref_t<std::tuple_element_t<0, T>>;
  return (std::same_as<
      first_element_type,
      std::remove_cvref_t<std::tuple_element_t<Is, T>>>
    && ...);
}(sequence_for<T>);

template<typename, tuple_like, typename>
struct apply_result;

template<typename F, tuple_like Tuple, std::size_t... Is>
struct apply_result<F, Tuple, constlist<Is...>> {
  using type = std::invoke_result_t<F, std::tuple_element_t<Is, Tuple>...>;
};

template<typename F, tuple_like Tuple>
using apply_result_t = typename apply_result<F, Tuple, decltype(sequence_for<Tuple>)>::type;

template<typename T, typename Tuple>
concept applicable = tuple_like<Tuple>
&& []<std::size_t... Is>(constlist<Is...>) {
  return std::invocable<T, std::tuple_element_t<Is, std::remove_reference_t<Tuple>>...>;
}(sequence_for<Tuple>);

template<typename T, typename Tuple, typename R>
concept applicable_r = applicable<T, Tuple> && std::convertible_to<apply_result_t<T, Tuple>, R>;

template<typename T, typename Tuple>
concept const_applicable = 
applicable<T, Tuple>
&& metavalue<apply_result_t<T, Tuple>>;

template<typename T, typename Tuple, typename R>
concept const_applicable_r =
const_applicable<T, Tuple>
&& std::convertible_to<decltype(apply_result_t<T, Tuple>::value), R>;

template<typename T, typename Tuple>
concept invocable_on_each = tuple_like<Tuple>
&& []<std::size_t... Is>(constlist<Is...>) {
  return (std::is_invocable_v<T &, std::tuple_element_t<Is, Tuple>> && ...);
}(sequence_for<Tuple>);

template<typename T, typename Tuple>
concept transformer_on_each = invocable_on_each<T, Tuple>
&& []<std::size_t... Is>(constlist<Is...>) {
  return !(std::same_as<std::invoke_result_t<T &, std::tuple_element_t<Is, Tuple>>, void> || ...);
}(sequence_for<Tuple>);

template<typename T, typename Tuple>
concept predicate_on_each = invocable_on_each<T, Tuple>
&& []<std::size_t... Is>(constlist<Is...>) {
  return (std::convertible_to<std::invoke_result_t<T &, std::tuple_element_t<Is, Tuple>>, bool> && ...);
}(sequence_for<Tuple>);

template<typename T, typename Tuple>
concept const_invocable_on_each =
transformer_on_each<T, Tuple>
&& []<std::size_t... Is>(constlist<Is...>) {
  return (metavalue<std::invoke_result_t<T &, std::tuple_element_t<Is, Tuple>>> && ...);
}(sequence_for<Tuple>);

template<typename T, typename Tuple>
concept const_predicate_on_each =
const_invocable_on_each<T, Tuple>
&& []<std::size_t... Is>(constlist<Is...>) {
  return (std::convertible_to< decltype(std::invoke_result_t<T &, std::tuple_element_t<Is, Tuple>>::value), bool> && ...);
}(sequence_for<Tuple>);

template<typename T, typename Init, typename Tuple>
concept left_foldable_on =
tuple_like<Tuple>
&& requires(T x, Init init, Tuple t) {
  []<std::size_t... Is>(T &&x, Init &&init, Tuple &&t, constlist<Is...>) {
    return (UPD_FWD(init), ..., accumulable(get<Is>(UPD_FWD(t)), x));
  }(UPD_FWD(x), UPD_FWD(init), UPD_FWD(t), sequence_for<Tuple>);
};

template<typename T, typename Init, typename Tuple>
concept right_foldable_on =
tuple_like<Tuple>
&& requires(T x, Init init, Tuple t) {
  []<std::size_t... Is>(T &&x, Init &&init, Tuple &&t, constlist<Is...>) {
    return (accumulable(get<Is>(UPD_FWD(t)), x), ..., UPD_FWD(init));
  }(UPD_FWD(x), UPD_FWD(t), sequence_for<Tuple>);
};

constexpr struct filter_void_t {} filter_void;

template<tuple_like Tuple>
[[nodiscard]] constexpr auto type_only(const Tuple &) noexcept(release);

template<typename Derived>
class tuple_implementation {
  template<typename Self>
  [[nodiscard]] constexpr auto derived(this Self &&self) noexcept(release) -> auto && {
    using retval_type = decltype(std::forward_like<Self>(std::declval<Derived>()));

    return static_cast<retval_type>(self);
  }

public:
  template<std::size_t I>
  using element_type = std::tuple_element_t<I, Derived>;

  template<typename Self, metavalue I>
  [[nodiscard]] constexpr auto operator[](this Self &&self, I i) noexcept(release) -> auto && {
    return UPD_FWD(self).at(i);
  }

  template<typename Self, metavalue I>
  [[nodiscard]] constexpr auto at(this Self &&self, I i) noexcept(release) -> auto && {
    return UPD_FWD(self).derived().template get<i>();
  }

  template<typename T, typename Self>
  [[nodiscard]] constexpr auto clean(this Self &&self, typebox<T>) noexcept(release) {
    using namespace std::ranges::views;

    constexpr auto truth_table = sequence<size()>
      .transform([](auto i) { return !std::same_as<element_type<i>, T>; });
    constexpr auto kept_index_count = truth_table.fold_left(0uz, plus);
    constexpr auto kept_indices = [&] {
      auto kept_indices = std::array<std::size_t, kept_index_count> {};
      auto i = 0uz;
      for (auto [j, keep] : truth_table.to_array() | enumerate) {
        if (keep) {
          kept_indices.at(i) = j;
          ++i;
        }
      }
      UPD_ASSERT(i == kept_indices.size());

      return kept_indices;
    }();

    return sequence<kept_indices.size()>
      .transform([&](auto i) { return expr<kept_indices.at(i)>; })
      .transform([&](auto i) -> auto&& { return UPD_FWD(self).at(i); });
  }

  template<typename Self>
  [[nodiscard]] constexpr auto clone(this Self &&self) {
    return UPD_FWD(self).transform([](auto &&x) { return UPD_FWD(x); });
  }

  template<const_predicate_on_each<Derived> UnaryPred> 
  [[nodiscard]] constexpr auto filter(UnaryPred &&p) const {
    struct cleanme_t {} cleanme;

    auto filter_one = [&](auto &&x) -> decltype(auto) {
      auto keep = UPD_INVOKE(p, std::as_const(x));
      if constexpr (keep) {
        return UPD_FWD(x);
      } else {
        return cleanme;
      }
    };

    return transform(filter_one).clean(typebox<cleanme_t>{});
  }

  template<auto Value> requires constlist_like<Derived>
  [[nodiscard]] constexpr auto find(auto_constant<Value>) const {
    return find_if([](const auto &x) { return expr<x.value == Value>; });
  }

  template<const_predicate_on_each<Derived> UnaryPred> 
  [[nodiscard]] constexpr auto find_if(UnaryPred &&p) const {
    auto filtered = zip(sequence<size()>, derived())
      .filter(unpack | [&](auto, const auto &x) { return UPD_INVOKE(p, x); })
      .clone();

    if constexpr (filtered.size() > 0) {
      return filtered[expr<0uz>][expr<0uz>];
    } else {
      return expr<size()>;
    }
  }

  template<typename Self>
  [[nodiscard]] constexpr auto flatten(this Self &&self) noexcept(release) requires nested_tuple<Derived> {
    using namespace std::ranges;
    namespace vw = std::ranges::views;

    using index2 = std::pair<std::size_t, std::size_t>;

    auto retval_size = self
      .transform([](auto sub) { return expr<sub.size()>; })
      .fold_left(expr<0uz>, plus);
    auto shape = self
      .transform([](auto sub) { return expr<sub.size()>; })
      .apply(to_metafunction([=](auto ...sizes) {
        auto shape = std::array<index2, retval_size>();
        auto size_array = std::array<std::size_t, sizeof...(sizes)> {sizes...};

        auto it = shape.begin();
        for (auto [i, size] : size_array | vw::enumerate) {
          auto make_i2 = [&](auto j) { return index2 {i, j}; };
          it = copy(vw::iota(0uz, size) | vw::transform(make_i2), it).out;
        }

        UPD_CONSTEXPR_ASSERT(it == shape.end());

        return shape;
      }));

      return sequence<shape.value.size()>
        .transform([=](auto i) {
          constexpr auto ij = shape.value.at(i);
          return std::pair{expr<ij.first>, expr<ij.second>};
        })
        .transform(unpack | [&](auto i, auto j) -> auto && { return UPD_FWD(self).at(i).at(j); })
      ;
  }

  template<typename Self, typename Init, typename BinaryOp>
  [[nodiscard]] constexpr auto fold_left(this Self &&self, Init &&init, BinaryOp op) noexcept(release) {
    static_assert(left_foldable_on<BinaryOp, Init, Derived>);

    auto impl = [&](auto... xs) {
      return (UPD_FWD(init), ..., accumulable(UPD_FWD(xs), op));
    };

    return UPD_FWD(self).apply(impl);
  }

  template<typename Self, typename Init, right_foldable_on<Init, Derived> BinaryOp>
  [[nodiscard]] constexpr auto fold_right(this Self &&self, Init &&init, BinaryOp op) noexcept(release) {
    static_assert(right_foldable_on<BinaryOp, Init, Derived>);

    auto impl = [&](auto... xs) {
      return (accumulable(UPD_FWD(xs), op), ..., UPD_FWD(init));
    };

    return UPD_FWD(self).apply(impl);
  }

  [[nodiscard]] constexpr static auto to_typelist() noexcept(release) requires typelist_like<Derived> {
    return Derived{}
    .apply([&](auto... metatypes) {
      using retval_type = typelist<typename decltype(metatypes)::type...>;
      return Derived::make_typelist(retval_type{});
    });
  }

  [[nodiscard]] constexpr static auto to_constlist() noexcept(release) requires constlist_like<Derived> {
    return Derived{}
    .apply([&](auto... metavalues) {
      using retval_type = constlist<metavalues.value...>;
      return Derived::make_constlist(retval_type{});
    });
  }

  [[nodiscard]] constexpr auto to_array() const noexcept(release) requires array_like<Derived> {
    return derived().apply([](auto &&... xs) {
      return std::array{UPD_FWD(xs)...};
    });
  }

  template<typename Self>
  [[nodiscard]] constexpr auto reverse(this Self &&self) noexcept(release)  requires typelist_like<Derived> {
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
      return UPD_FWD(self).at(expr<rindices[i]>);
    };

    return sequence<self.size()>.transform(get_element);
  }

  template<typename Self>
  [[nodiscard]] constexpr auto square(this Self &&self) noexcept(release) {
    auto seq = sequence<Derived::size()>;
    
    auto make_ipair_list = [=](auto i) {
      return seq.transform([&](auto j) { return constlist<i, j>{}; });
    };

    auto make_pair = [&](auto ipair) {
      auto [i, j] = ipair;
      return tuple{
        ref{UPD_FWD(self).at(i)},
        ref{UPD_FWD(self).at(j)}
      };
    };

    return seq
      .transform(make_ipair_list)
      .flatten()
      .transform(make_pair);
  }

  template<typename Self, typename F>
  [[nodiscard]] constexpr auto transform(this Self &&self, F &&f) {
    UPD_THIS_DEDUCTION_REQUIRES_WORKAROUND(transformer_on_each<F, Derived>);

    auto invoke_f_at = [&](auto i) -> decltype(auto) {
      return f(UPD_FWD(self).at(i));
    };

    auto invoke_f_on_each = [&](auto ...is) {
      auto element_types = typelist<decltype(invoke_f_at(is))...>{};
      return UPD_FWD(self)
        .derived()
        .make_tuple(element_types, invoke_f_at(is)...);
    };
    
    return sequence<size()>.apply(invoke_f_on_each);
  }

  template<typename Self, invocable_on_each<Derived> F>
  [[nodiscard]] constexpr auto transform(this Self &&self, F &&f, filter_void_t) {
    struct cleanme_t {} cleanme;

    auto invoke_f_or_mark = [&]<typename T>(T &&x) -> decltype(auto) {
      if constexpr (std::is_void_v<std::invoke_result_t<F &, T>>) {
        return cleanme;
      } else {
        return UPD_INVOKE(f, UPD_FWD(x));
      }
    };

    return UPD_FWD(self).transform(invoke_f_or_mark).clean(typebox<cleanme_t>{});
  }

  [[nodiscard]] constexpr auto type_only() const noexcept(release) {
    return upd::type_only(derived());
  }

  template<typename Self, typename F>
  [[nodiscard]] constexpr auto apply(this Self &&self, F &&f) -> decltype(auto) {
    // UPD_THIS_DEDUCTION_REQUIRES_WORKAROUND(applicable<F, Derived>);
    auto seq = std::make_index_sequence<size()>{};

    return [&]<std::size_t ...Is>(std::index_sequence<Is...>) -> decltype(auto) {
      return UPD_INVOKE(UPD_FWD(f), UPD_FWD(self).at(expr<Is>)...);
    } (seq);
  }

  template<typename Self, typename F>
  constexpr void for_each(this Self &&self, F &&f) {
    static_assert(invocable_on_each<F, Derived>);

    UPD_FWD(self).apply([&](auto &&... xs) {
      ((void) UPD_INVOKE(f, UPD_FWD(xs)), ...);
    });
  }

  [[nodiscard]] constexpr static auto size() noexcept(release) -> std::size_t {
    return std::tuple_size_v<Derived>;
  }
};

template<typename... Ts>
class tuple : public tuple_implementation<tuple<Ts...>> {
  using leaves = detail::leaves<std::index_sequence_for<Ts...>, Ts...>;

public:
  template<typename... Us, typename... Args>
  [[nodiscard]] constexpr static auto make_tuple(typelist<Us...>, Args &&... xs) {
    return tuple<Us...>{UPD_FWD(xs)...};
  }

  template<typename... Us>
  [[nodiscard]] constexpr static auto make_typelist(typelist<Us...>) noexcept(release) {
    return typelist<Us...>{};
  }

  template<auto... Values>
  [[nodiscard]] constexpr static auto make_constlist(constlist<Values...>) noexcept(release) {
    return constlist<Values...>{};
  }

  template<typename ...Us>
  constexpr explicit tuple(Us &&... xs): m_leaves{UPD_FWD(xs)...} {}

  template<typename ...Us>
  constexpr explicit tuple(std::in_place_t, Us &&... xs): m_leaves{UPD_FWD(xs)...} {}

  template<std::size_t I, typename Self>
  [[nodiscard]] constexpr auto get(this Self &&self) noexcept(release) -> auto && {
    decltype(auto) retval = UPD_FWD(self).m_leaves.at(expr<I>);

    using retval_type = decltype(retval);
    static_assert(!std::is_same_v<retval_type, detail::variadic::not_found_t>, "`I` is not a valid index for `tuple`");

    return UPD_FWD(retval);
  }

private:
  leaves m_leaves;
};

template<typename ...Ts>
explicit tuple(Ts...) -> tuple<
  typename std::conditional_t<instance_of<Ts, ref>, Ts, std::type_identity<Ts>>::type...
>;

template<typename ...Ts>
explicit tuple(std::in_place_t, Ts...) -> tuple<
  typename std::conditional_t<instance_of<Ts, ref>, Ts, std::type_identity<Ts>>::type...
>;

template<typename... Ts>
class typelist : public tuple_implementation<typelist<Ts...>> {
  using leaves = detail::leaves<std::index_sequence_for<Ts...>, typebox<Ts>...>;

public:
  template<typename... Us, typename... Args>
  [[nodiscard]] constexpr static auto make_tuple(typelist<Us...>, Args &&... xs) {
    return tuple<Us...>{UPD_FWD(xs)...};
  }

  template<typename... Us>
  [[nodiscard]] constexpr static auto make_typelist(typelist<Us...>) noexcept(release) {
    return typelist<Us...>{};
  }

  template<auto... Values>
  [[nodiscard]] constexpr static auto make_constlist(constlist<Values...>) noexcept(release) {
    return constlist<Values...>{};
  }

  constexpr typelist() noexcept(release) = default;

  template<metatype... Metas>
  constexpr explicit typelist(Metas...) noexcept(release) {}

  template<std::size_t I, typename Self>
  [[nodiscard]] constexpr auto get(this Self &&self) noexcept(release) -> auto && {
    decltype(auto) retval = UPD_FWD(self).m_leaves.at(expr<I>);

    using retval_type = decltype(retval);
    static_assert(!std::is_same_v<retval_type, detail::variadic::not_found_t>, "`I` is not a valid index for `tuple`");

    return UPD_FWD(retval);
  }

private:
  leaves m_leaves;
};

template<metatype... Metas>
explicit typelist(Metas...) -> typelist<
  typename Metas::type...
>;

template<auto... Vs>
class constlist : public tuple_implementation<constlist<Vs...>> {
  template<std::size_t, typename Tuple>
  friend constexpr auto get(Tuple &&) noexcept(release) -> auto &&;

  using leaves = detail::leaves<std::make_index_sequence<sizeof...(Vs)>, auto_constant<Vs>...>;

public:
  template<typename... Ts, typename... Args>
  [[nodiscard]] constexpr static auto make_tuple(typelist<Ts...>, Args &&... xs) {
    return tuple<Ts...>{UPD_FWD(xs)...};
  }

  template<typename... Us>
  [[nodiscard]] constexpr static auto make_typelist(typelist<Us...>) noexcept(release) {
    return typelist<Us...>{};
  }

  template<auto... Values>
  [[nodiscard]] constexpr static auto make_constlist(constlist<Values...>) noexcept(release) {
    return constlist<Values...>{};
  }

  constexpr constlist() noexcept(release) = default;

  template<metavalue... Metas>
  constexpr explicit constlist(Metas...) noexcept(release) {}

  template<std::size_t I, typename Self>
  [[nodiscard]] constexpr auto get(this Self &&self) noexcept(release) -> auto && {
    decltype(auto) retval = UPD_FWD(self).m_leaves.at(expr<I>);

    using retval_type = decltype(retval);
    static_assert(!std::is_same_v<retval_type, detail::variadic::not_found_t>, "`I` is not a valid index for `tuple`");

    return UPD_FWD(retval);
  }

private:
  leaves m_leaves;
};

template<metavalue... Metas>
explicit constlist(Metas...) -> constlist<
  Metas::value...
>;

template<tuple_like Tuple>
[[nodiscard]] constexpr auto type_only(const Tuple &) noexcept(release) {
  constexpr auto size = std::tuple_size_v<Tuple>;

  return sequence<size>
    .transform([](auto i) { return typebox<std::tuple_element_t<i, Tuple>>{}; })
    .apply([](auto ...types) { return typelist {types...}; });
}

template<tuple_like ...Tuples> requires (!UPD_VARIADIC_CONSTRAINT_WORKAROUND(typelist_like<Tuples> || named_tuple_like<Tuples>))
[[nodiscard]] constexpr auto concat(Tuples &&... ts) noexcept(release) {
  auto element_types = concat(type_only(ts)...);
  using retval_type = instantiate_variadic<tuple, decltype(element_types)>;

  return tuple{UPD_FWD(ts)...}
    .flatten()
    .apply([](auto &&... xs) { return retval_type { UPD_FWD(xs)... }; });
}

template<typelist_like ...Typelists>
[[nodiscard]] constexpr auto concat(Typelists... tls) noexcept(release) {
  return tuple{tls...}
  .flatten()
  .clone()
  .to_typelist();
}

template<constlist_like ...Constlists>
[[nodiscard]] constexpr auto concat(Constlists... cls) noexcept(release) {
  return tuple{cls...}
  .flatten()
  .clone()
  .to_constlist();
}

template<tuple_like Lhs, tuple_like Rhs>
[[nodiscard]] constexpr auto operator+(Lhs &&lhs, Rhs &&rhs) {
  return concat(UPD_FWD(lhs), UPD_FWD(rhs));
}

template<tuple_like... Tuples>
[[nodiscard]] constexpr auto zip(Tuples &&... ts) {
  constexpr auto min_size = std::min(ts.size()...);

  return sequence<min_size>
    .transform([&](auto i) {
        return tuple{ref{UPD_FWD(ts).at(i)}...};
    });
}

template<typename F>
struct unpacker {
  template<typename Self, typename Tuple>
  [[nodiscard]] constexpr auto operator()(this Self &&self, Tuple &&t) noexcept(release) -> decltype(auto) {
    UPD_THIS_DEDUCTION_REQUIRES_WORKAROUND(applicable<F, Tuple>);

    using noref_type = std::remove_reference_t<Tuple>;
    constexpr auto size = std::tuple_size_v<noref_type>;
    
    return [&]<auto... Is>(constlist<Is...>) -> decltype(auto) {
      return UPD_INVOKE(UPD_FWD(self).invocable, get<Is>(UPD_FWD(t))...);
    }(sequence<size>);
  }

  F invocable;
};

template<typename F>
[[nodiscard]] constexpr auto operator|(unpack_t, F &&f) {
  return unpacker{UPD_FWD(f)};
}

} // namespace upd
