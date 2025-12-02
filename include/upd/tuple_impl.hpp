#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <ranges>
#include <type_traits>
#include <utility>

#include "constexpr.hpp"
#include "functional.hpp"
#include "ref.hpp"
#include "tuple/tuple_like.hpp"
#include "tuple/tuple_size.hpp"
#include "type_traits.hpp"
#include "upd.hpp"
#include "with_sequence.hpp"

namespace upd {

template<typename Tuple>
concept tuple_like = tuple_like2<Tuple>;

template<typename Typelist>
concept typelist_like = tuple_like<Typelist> && UPD_WITH_SEQUENCE_FOR(Is, Typelist) {
  using noref_type [[maybe_unused]] = std::remove_reference_t<Typelist>;
  return (metatype<std::tuple_element_t<Is, noref_type>> && ...);
};

template<typename Constlist>
concept constlist_like = tuple_like<Constlist> && UPD_WITH_SEQUENCE_FOR(Is, Constlist) {
  using noref_type [[maybe_unused]] = std::remove_reference_t<Constlist>;
  return (metavalue<std::remove_cvref_t<std::tuple_element_t<Is, noref_type>>> && ...);
};

template<typename Array>
concept array_like = tuple_like<Array> && std::tuple_size_v<Array> > 0 && UPD_WITH_SEQUENCE_FOR(Is, Array) {
  using first_element_type = std::remove_cvref_t<std::tuple_element_t<0, Array>>;
  return (std::same_as<first_element_type, std::remove_cvref_t<std::tuple_element_t<Is, Array>>> && ...);
};

template<typename Tuple>
concept nested_tuple = tuple_like<Tuple> && UPD_WITH_SEQUENCE_FOR(Is, Tuple) {
  using noref_type [[maybe_unused]] = std::remove_reference_t<Tuple>;
  return (tuple_like<std::tuple_element_t<Is, noref_type>> && ...);
};

template<typename, typename, typename>
struct apply_result;

template<typename F, typename Tuple, std::size_t... Is>
struct apply_result<F, Tuple, std::index_sequence<Is...>> {
  using type = std::invoke_result_t<F, std::tuple_element_t<Is, Tuple>...>;
};

template<typename F, tuple_like Tuple>
using apply_result_t = typename apply_result<F, Tuple, std::make_index_sequence<std::tuple_size_v<Tuple>>>::type;

template<typename F, typename Tuple>
concept applicable = tuple_like<Tuple> && UPD_WITH_SEQUENCE_FOR(Is, Tuple) {
  return invocable<F, std::tuple_element_t<Is, std::remove_reference_t<Tuple>>...>;
};

template<typename F, typename Tuple, typename R>
concept applicable_r = applicable<F, Tuple> && std::convertible_to<apply_result_t<F, Tuple>, R>;

template<typename F, typename Tuple>
concept const_applicable = applicable<F, Tuple> && metavalue<apply_result_t<F, Tuple>>;

template<typename F, typename Tuple, typename R>
concept const_applicable_r =
    const_applicable<F, Tuple> && std::convertible_to<decltype(apply_result_t<F, Tuple>::value), R>;

template<typename F, typename Tuple>
concept invocable_on_each = tuple_like<Tuple> && UPD_WITH_SEQUENCE_FOR(Is, Tuple) {
  return (invocable<F &, std::tuple_element_t<Is, Tuple>> && ...);
};

template<typename F, typename Tuple>
concept transformer_on_each = invocable_on_each<F, Tuple> && UPD_WITH_SEQUENCE_FOR(Is, Tuple) {
  return !(std::same_as<std::invoke_result_t<F &, std::tuple_element_t<Is, Tuple>>, void> || ...);
};

template<typename F, typename Tuple>
concept predicate_on_each = invocable_on_each<F, Tuple> && UPD_WITH_SEQUENCE_FOR(Is, Tuple) {
  return (std::convertible_to<std::invoke_result_t<F &, std::tuple_element_t<Is, Tuple>>, bool> && ...);
};

template<typename F, typename Tuple>
concept const_invocable_on_each = transformer_on_each<F, Tuple> && UPD_WITH_SEQUENCE_FOR(Is, Tuple) {
  return (metavalue<std::invoke_result_t<F &, std::tuple_element_t<Is, Tuple>>> && ...);
};

template<typename F, typename Tuple>
concept const_predicate_on_each = const_invocable_on_each<F, Tuple> && predicate_on_each<F, Tuple>;

template<typename BinaryOp, typename Init, typename Tuple>
concept left_foldable_on = tuple_like<Tuple> && requires(BinaryOp op, Init init, Tuple t) {
  UPD_WITH_SEQUENCE_FOR(Is, Tuple, op, init, t) { (void)(UPD_FWD(init), ..., accumulable(get<Is>(UPD_FWD(t)), op)); };
};

template<typename BinaryOp, typename Init, typename Tuple>
concept right_foldable_on = tuple_like<Tuple> && requires(BinaryOp op, Init init, Tuple t) {
  UPD_WITH_SEQUENCE_FOR(Is, Tuple, op, init, t) { (void)(accumulable(get<Is>(UPD_FWD(t)), op), ..., UPD_FWD(init)); };
};

constexpr struct filter_void_t {
} filter_void;

template<typename... Ts>
class tuple; // IWYU pragma: keep

template<typename... Ts>
class typelist;

template<auto... Xs>
class constlist;

template<std::size_t N>
constexpr auto sequence =
    []<std::size_t... Is>(std::index_sequence<Is...>) { return constlist<Is...>{}; }(std::make_index_sequence<N>{});

template<tuple_like Tuple>
constexpr auto sequence_for = sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>;

template<typename F>
struct unpacker {
  template<typename Self, typename Tuple>
    requires applicable<F, Tuple>
  [[nodiscard]] constexpr auto operator()(this Self &&self, Tuple &&t) noexcept(release) -> decltype(auto) {
    using noref_type = std::remove_reference_t<Tuple>;
    constexpr auto size = std::tuple_size_v<noref_type>;

    return [&]<auto... Is>(constlist<Is...>) -> decltype(auto) {
      return UPD_INVOKE(UPD_FWD(self).invocable, get<Is>(UPD_FWD(t))...);
    }(sequence<size>);
  }

  F invocable;
};

constexpr struct unpack_t {
} unpack;

template<typename F>
[[nodiscard]] constexpr auto operator|(unpack_t, F &&f) {
  return unpacker{UPD_FWD(f)};
}

template<tuple_like Tuple>
[[nodiscard]] constexpr auto type_only(const Tuple &) noexcept(release) {
  constexpr auto size = std::tuple_size_v<Tuple>;

  return sequence<size>.transform([](auto i) { return typebox<std::tuple_element_t<i, Tuple>>{}; }).apply([](auto... types) {
    return typelist{types...};
  });
}

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

  /* Compiler bug with gcc14
  template<typename Self, metavalue I>
  [[nodiscard]] constexpr auto at(this Self &&self, I i) noexcept(release) -> auto && {
    return UPD_FWD(self).derived().template get<i>();
  }
  */

  template<metavalue I>
  [[nodiscard]] constexpr auto at(I i) & noexcept(release) -> auto & {
    return derived().template get<i>();
  }

  template<metavalue I>
  [[nodiscard]] constexpr auto at(I i) const & noexcept(release) -> const auto & {
    return derived().template get<i>();
  }

  template<metavalue I>
  [[nodiscard]] constexpr auto at(I i) && noexcept(release) -> auto && {
    return std::move(derived()).template get<i>();
  }

  template<metavalue I>
  [[nodiscard]] constexpr auto at(I i) const && noexcept(release) -> const auto && {
    return std::move(derived()).template get<i>();
  }

  template<typename Self, typename... NewValues>
  [[nodiscard]] constexpr auto chain_before(this Self &&self, NewValues &&...new_values) noexcept(release) {
    return UPD_FWD(self).apply([&](auto &&...xs) { return tuple{UPD_FWD(new_values)..., UPD_FWD(xs)...}; });
  }

  template<typename T, typename Self>
  [[nodiscard]] constexpr auto clean(this Self &&self, typebox<T>) noexcept(release) {
    using namespace std::ranges::views;

    constexpr auto truth_table = sequence<size()>.transform([](auto i) { return !std::same_as<element_type<i>, T>; });
    constexpr auto kept_index_count = truth_table.fold_left(0uz, plus);
    constexpr auto kept_indices = [&] {
      auto kept_indices = std::array<std::size_t, kept_index_count>{};
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

    return sequence<kept_indices.size()>.transform([&](auto i) { return expr<kept_indices.at(i)>; }).transform([&](auto i) -> auto && {
      return UPD_FWD(self).at(i);
    });
  }

  template<typename Self>
  [[nodiscard]] constexpr auto clone(this Self &&self) {
    return UPD_FWD(self).transform([](auto &&x) { return UPD_FWD(x); });
  }

  template<const_predicate_on_each<Derived> UnaryPred>
  [[nodiscard]] constexpr auto filter(UnaryPred &&p) const {
    struct cleanme_t {
    } cleanme;

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

  template<auto Value>
    requires constlist_like<Derived>
  [[nodiscard]] constexpr auto find(auto_constant<Value>) const {
    return find_if([](const auto &x) {
      constexpr auto are_comparable = (requires { x.value == Value; });
      if constexpr (are_comparable) {
        return expr<x.value == Value>;
      } else {
        return expr<false>;
      }
    });
  }

  template<typename T>
  [[nodiscard]] constexpr auto find(const T &value) const {
    return find_if([&](const auto &x) {
      constexpr auto are_comparable = (requires { x == value; });
      if constexpr (are_comparable) {
        return x == value;
      } else {
        return false;
      }
    });
  }

  template<typename UnaryPred>
  [[nodiscard]] constexpr auto find_if(UnaryPred &&p) const -> std::size_t {
    auto retval = 0uz;
    auto found_yet = false;

    for_each([&](const auto &x) {
      if (found_yet) {
        return;
      }

      if (UPD_INVOKE(p, x)) {
        found_yet = true;
      } else {
        ++retval;
      }
    });

    return retval;
  }

  template<const_predicate_on_each<Derived> UnaryPred>
  [[nodiscard]] constexpr auto find_if(UnaryPred &&p) const {
    auto filtered =
        zip(sequence<size()>, derived()).filter(unpack | [&](auto, const auto &x) { return UPD_INVOKE(p, x); }).clone();

    if constexpr (filtered.size() > 0) {
      return filtered[expr<0uz>][expr<0uz>];
    } else {
      return expr<size()>;
    }
  }

  template<typename Self>
  [[nodiscard]] constexpr auto flatten(this Self &&self) noexcept(release)
    requires nested_tuple<Derived>
  {
    using namespace std::ranges;
    namespace vw = std::ranges::views;

    using index2 = std::pair<std::size_t, std::size_t>;

    auto retval_size =
        self.transform([](auto sub) { return expr<tuple_size_v<decltype(sub)>>; }).fold_left(expr<0uz>, plus);
    auto shape = self.transform([](auto sub) { return expr<tuple_size_v<decltype(sub)>>; })
                     .apply(to_metafunction([=](auto... sizes) {
                       auto shape = std::array<index2, retval_size>();
                       auto size_array = std::array<std::size_t, sizeof...(sizes)>{sizes...};

                       auto it = shape.begin();
                       for (auto [i, size] : size_array | vw::enumerate) {
                         for (auto j = 0zu; j < size; ++j) {
                           *it++ = index2{i, j};
                         }
                       }

                       UPD_CONSTEXPR_ASSERT(it == shape.end());

                       return shape;
                     }));

    return sequence<shape.value.size()>
        .transform([=](auto i) {
          constexpr auto ij = shape.value.at(i);
          return std::pair{expr<ij.first>, expr<ij.second>};
        })
        .transform(unpack | [&](auto i, auto j) -> decltype(auto) { return get<j>(UPD_FWD(self).at(i)); })
      ;
  }

  template<typename Self, typename Init, typename BinaryOp>
  [[nodiscard]] constexpr auto fold_left(this Self &&self, Init &&init, BinaryOp op) noexcept(release) {
    static_assert(left_foldable_on<BinaryOp, Init, Derived>);

    auto impl = [&](auto... xs) { return (UPD_FWD(init), ..., accumulable(UPD_FWD(xs), op)); };

    return UPD_FWD(self).apply(impl);
  }

  template<typename Self, typename Init, right_foldable_on<Init, Derived> BinaryOp>
  [[nodiscard]] constexpr auto fold_right(this Self &&self, Init &&init, BinaryOp op) noexcept(release) {
    static_assert(right_foldable_on<BinaryOp, Init, Derived>);

    auto impl = [&](auto... xs) { return (accumulable(UPD_FWD(xs), op), ..., UPD_FWD(init)); };

    return UPD_FWD(self).apply(impl);
  }

  [[nodiscard]] constexpr static auto to_typelist() noexcept(release)
    requires typelist_like<Derived>
  {
    return Derived{}.apply([&](auto... metatypes) {
      using retval_type = typelist<typename decltype(metatypes)::type...>;
      return Derived::make_typelist(retval_type{});
    });
  }

  [[nodiscard]] constexpr static auto to_constlist() noexcept(release)
    requires constlist_like<Derived>
  {
    return Derived{}.apply([&](auto... metavalues) {
      using retval_type = constlist<metavalues.value...>;
      return Derived::make_constlist(retval_type{});
    });
  }

  [[nodiscard]] constexpr auto to_array() const noexcept(release)
    requires array_like<Derived>
  {
    return derived().apply([](auto &&...xs) { return std::array{UPD_FWD(xs)...}; });
  }

  template<typename Self>
  [[nodiscard]] constexpr auto reverse(this Self &&self) noexcept(release)
    requires typelist_like<Derived>
  {
    namespace stdr = std::ranges;

    constexpr auto rindices = []() {
      auto rindices = std::array<std::size_t, Derived::size()>{};
      auto first = rindices.rbegin();
      auto last = rindices.rend();
      auto i = std::size_t{0};
      for (auto &ri : stdr::subrange{first, last}) {
        ri = i++;
      }

      return rindices;
    }();

    auto get_element = [&](auto i) -> auto && { return UPD_FWD(self).at(expr<rindices[i]>); };

    return sequence<self.size()>.transform(get_element);
  }

  template<typename Self>
  [[nodiscard]] constexpr auto square(this Self &&self) noexcept(release) {
    auto seq = sequence<Derived::size()>;

    auto make_ipair_list = [=](auto i) {
      return seq.transform([&](auto j) { return constlist<std::size_t{i}, std::size_t{j}>{}; });
    };

    auto make_pair = [&](auto ipair) {
      const auto &[i, j] = ipair;
      return tuple{ref{UPD_FWD(self).at(i)}, ref{UPD_FWD(self).at(j)}};
    };

    return seq.transform(make_ipair_list).flatten().transform(make_pair);
  }

  template<typename Self, typename F>
  [[nodiscard]] constexpr auto transform(this Self &&self, F &&f) {
    // UPD_THIS_DEDUCTION_REQUIRES_WORKAROUND(transformer_on_each<F, Derived>);

    auto invoke_f_at = [&](auto i) -> decltype(auto) { return f(UPD_FWD(self).at(i)); };

    auto invoke_f_on_each = [&](auto... is) {
      auto element_types = typelist<decltype(invoke_f_at(is))...>{};
      return UPD_FWD(self).derived().make_tuple(element_types, invoke_f_at(is)...);
    };

    return sequence<size()>.apply(invoke_f_on_each);
  }

  template<typename Self, invocable_on_each<Derived> F>
  [[nodiscard]] constexpr auto transform(this Self &&self, F &&f, filter_void_t) {
    struct cleanme_t {
    } cleanme;

    auto invoke_f_or_mark = [&]<typename T>(T &&x) -> decltype(auto) {
      if constexpr (std::is_void_v<std::invoke_result_t<F &, T>>) {
        return cleanme;
      } else {
        return UPD_INVOKE(f, UPD_FWD(x));
      }
    };

    return UPD_FWD(self).transform(invoke_f_or_mark).clean(typebox<cleanme_t>{});
  }

  [[nodiscard]] constexpr auto type_only() const noexcept(release) { return upd::type_only(derived()); }

  template<typename Self, typename F>
    requires applicable<F, Derived>
  [[nodiscard]] constexpr auto apply(this Self &&self, F &&f) -> decltype(auto) {
    auto seq = std::make_index_sequence<size()>{};

    return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> decltype(auto) {
      return UPD_INVOKE(UPD_FWD(f), UPD_FWD(self).at(expr<Is>)...);
    }(seq);
  }

  template<typename Self, typename F>
  constexpr void for_each(this Self &&self, F &&f) {
    static_assert(invocable_on_each<F, Derived>);

    UPD_FWD(self).apply([&](auto &&...xs) { ((void)UPD_INVOKE(f, UPD_FWD(xs)), ...); });
  }

  [[nodiscard]] constexpr static auto size() noexcept(release) -> std::size_t { return std::tuple_size_v<Derived>; }

  template<typename Self, typename F>
  [[nodiscard]] constexpr auto visit(this Self &&self, std::size_t i, F &&f) {
    using retval_type = typename decltype(self.type_only().apply([]<typename... Ts>(typebox<Ts>...) {
      using retval_type = std::common_type_t<std::invoke_result_t<F, Ts>...>;
      return typebox<retval_type>{};
    }))::type;

    UPD_ASSERT(i < size());

    auto make_invoker_for_ith = [&](auto i) {
      return +[](Self &&self, F &&f) -> retval_type { return UPD_INVOKE(UPD_FWD(f), UPD_FWD(self)[decltype(i){}]); };
    };
    auto lut = sequence<size()>.transform(make_invoker_for_ith).to_array();

    return UPD_INVOKE(lut[i], UPD_FWD(self), UPD_FWD(f));
  }
};

template<tuple_like... Tuples>
[[nodiscard]] constexpr auto concat(Tuples &&...ts) noexcept(release) {
  return tuple{std::in_place, UPD_FWD(ts)...}.flatten().apply([](auto &&...xs) { return tuple{UPD_FWD(xs)...}; });
}

template<tuple_like Lhs, tuple_like Rhs>
[[nodiscard]] constexpr auto operator+(Lhs &&lhs, Rhs &&rhs) {
  return concat(UPD_FWD(lhs), UPD_FWD(rhs));
}

template<tuple_like... Tuples>
[[nodiscard]] constexpr auto zip(Tuples &&...ts) {
  constexpr auto min_size = std::min({ts.size()...});

  return sequence<min_size>.transform([&](auto i) { return tuple{ref{UPD_FWD(ts).at(i)}...}; });
}

template<tuple_like... Tuples>
[[nodiscard]] constexpr auto join(Tuples &&...ts) noexcept(release) {
  auto retval = tuple{std::in_place, ref{UPD_FWD(ts)}...}.flatten();

  return retval;
}

} // namespace upd
