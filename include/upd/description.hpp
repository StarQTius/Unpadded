#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <tuple>
#include <type_traits>
#include <variant>

#include "detail/always_false.hpp"
#include "detail/has_value_member.hpp"
#include "detail/integral_constant.hpp"
#include "detail/variadic/equals.hpp"
#include "detail/variadic/filter.hpp"
#include "detail/variadic/map.hpp"
#include "detail/variadic/product.hpp"
#include "detail/variadic/to_array.hpp"
#include "integer.hpp"
#include "named_value.hpp"
#include "token.hpp"
#include "tuple.hpp"
#include "upd.hpp"

namespace upd::descriptor {

template<name, bool, std::size_t>
struct field_t;

template<name, std::size_t>
struct constant_t;

} // namespace upd::descriptor

namespace upd::detail {

template<typename Iter>
class iterator_reference {
public:
  using iterator_type = Iter;
  using difference_type = typename std::iterator_traits<Iter>::difference_type;
  using value_type = typename std::iterator_traits<Iter>::value_type;
  using pointer = typename std::iterator_traits<Iter>::pointer;
  using reference = typename std::iterator_traits<Iter>::reference;
  using iterator_category = std::input_iterator_tag;
  
  constexpr explicit iterator_reference(iterator_type &iter) : m_cache{*iter}, m_iter{iter} {
    ++iter;
  }

  [[nodiscard]] constexpr auto operator*() const -> value_type { 
    return m_cache;
  }

  constexpr auto operator++() -> iterator_reference & {
    m_cache = *m_iter;
    ++m_iter.get();
    return *this;
  }

  constexpr auto operator++() const -> const iterator_reference & {
    m_cache = *m_iter;
    ++m_iter.get();
    return *this;
  }

private:
  std::reference_wrapper<iterator_type> m_iter;
  value_type m_cache;
};

} // namespace upd::detail

namespace upd {

template<typename T, template<typename...> typename TT>
concept instance_of = detail::is_instance_of_v<T, TT>;

template<typename>
class invoker_iterator;

template<typename Parent>
requires instance_of<std::remove_cv_t<Parent>, invoker_iterator>
class invoker_iterator_proxy;

template<typename F>
class invoker_iterator {
  template<typename Parent>
  requires instance_of<std::remove_cv_t<Parent>, invoker_iterator>
  friend class invoker_iterator_proxy;

public:
  using invocable_type = F;

  constexpr explicit invoker_iterator(F f) noexcept : m_f{std::move(f)} {}

  constexpr auto operator*() noexcept -> invoker_iterator_proxy<invoker_iterator> {
    return invoker_iterator_proxy{this};
  }

  constexpr auto operator*() const noexcept -> invoker_iterator_proxy<const invoker_iterator> {
    return invoker_iterator_proxy{this};
  }

  constexpr auto operator++() noexcept -> invoker_iterator & {
    return *this;
  }

  constexpr auto operator++() const noexcept -> const invoker_iterator & {
    return *this;
  }

private:
  F m_f;
};

template<typename Parent>
requires instance_of<std::remove_cv_t<Parent>, invoker_iterator>
class invoker_iterator_proxy {
  template<typename>
  friend class invoker_iterator;

public:
  using invocable_type = typename Parent::invocable_type;

  template<typename T> requires std::invocable<invocable_type, T>
  constexpr auto operator=(T &&x) const -> const invoker_iterator_proxy & {
    std::invoke(m_parent->m_f, UPD_FWD(x));
    return *this;
  }

private:
  constexpr invoker_iterator_proxy(Parent *parent) noexcept: m_parent{parent} {}

  Parent *m_parent;
};

template<typename Iter, typename F>
class transformer_iterator {
public:
  using difference_type = typename std::iterator_traits<Iter>::difference_type;
  using value_type = std::remove_cvref_t<std::invoke_result_t<F, typename std::iterator_traits<Iter>::value_type>>;
  using reference = std::invoke_result_t<F, typename std::iterator_traits<Iter>::value_type> &;
  using pointer = decltype(&std::declval<reference>());
  using iterator_category = std::input_iterator_tag;

  constexpr explicit transformer_iterator(Iter iter, F f) noexcept:
    m_iter{iter},
    m_f{std::move(f)}
  {}

  [[nodiscard]] constexpr auto operator*() -> decltype(auto) {
    return m_f(*m_iter);
  }

  constexpr auto operator++() -> transformer_iterator & {
    ++m_iter;
    return *this;
  }

private:
  Iter m_iter;
  F m_f;
};

enum class error {
  none,
  checksum_mismatch,
  constant_mismatch,
};

template<typename E = error>
class unexpected {
public:
  constexpr unexpected() noexcept = default;

  constexpr unexpected(E e) noexcept : m_error{std::move(e)} {}

  [[nodiscard]] constexpr operator bool() const noexcept { return static_cast<bool>(m_error); }

  [[nodiscard]] constexpr auto error() const noexcept -> E { return m_error; }

private:
  E m_error;
};

template<typename T, typename E = error>
class expected {
public:
  constexpr explicit expected(T x) noexcept : m_value_or_error{std::in_place_index<0>, std::move(x)} {}

  constexpr expected(unexpected<E> e) noexcept : m_value_or_error{std::in_place_index<1>, e.error()} {}

  [[nodiscard]] constexpr operator bool() const noexcept { return value_if() != nullptr; }

  [[nodiscard]] constexpr auto operator*() noexcept -> T & { return value(); }

  [[nodiscard]] constexpr auto operator*() const noexcept -> const T & { return value(); }

  [[nodiscard]] constexpr auto operator->() noexcept -> T * { return &value(); }

  [[nodiscard]] constexpr auto operator->() const noexcept -> const T * { return &value(); }

  [[nodiscard]] constexpr auto value() &noexcept -> T & {
    auto *v = value_if();
    UPD_ASSERT(v != nullptr);

    return *v;
  }

  [[nodiscard]] constexpr auto value() const &noexcept -> const T & {
    const auto *v = value_if();
    UPD_ASSERT(v != nullptr);

    return *v;
  }

  [[nodiscard]] constexpr auto value() &&noexcept -> T && {
    auto *v = value_if();
    UPD_ASSERT(v != nullptr);

    return std::move(*v);
  }

  [[nodiscard]] constexpr auto error() &noexcept -> E & {
    auto *e = error_if();
    UPD_ASSERT(e != nullptr);

    return *e;
  }

  [[nodiscard]] constexpr auto error() const &noexcept -> const E & {
    const auto *e = error_if();
    UPD_ASSERT(e != nullptr);

    return *e;
  }

  [[nodiscard]] constexpr auto error() &&noexcept -> E && {
    auto *e = error_if();
    UPD_ASSERT(e != nullptr);

    return std::move(*e);
  }

  [[nodiscard]] constexpr auto value_if() noexcept -> T * { return std::get_if<0>(&m_value_or_error); }

  [[nodiscard]] constexpr auto value_if() const noexcept -> const T * { return std::get_if<0>(&m_value_or_error); }

  [[nodiscard]] constexpr auto error_if() noexcept -> E * { return std::get_if<1>(&m_value_or_error); }

  [[nodiscard]] constexpr auto error_if() const noexcept -> const E * { return std::get_if<1>(&m_value_or_error); }

private:
  std::variant<T, E> m_value_or_error;
};

} // namespace upd

namespace upd::descriptor {

struct all_fields_t {};

constexpr auto all_fields = all_fields_t{};

template<name, bool Is_Signed, std::size_t Width>
constexpr auto field(signedness_t<Is_Signed>, width_t<Width>) noexcept(release);

template<name, typename T, std::size_t Width>
constexpr auto constant(T, width_t<Width>) noexcept(release);

template<name, typename BinaryOp, std::size_t Width, typename Init>
constexpr auto checksum(BinaryOp, width_t<Width>, Init, all_fields_t) noexcept(release);

} // namespace upd::descriptor

namespace upd {

template<typename Range>
[[nodiscard]] constexpr auto all_of(const Range &range) noexcept -> bool {
  for (const auto &e : range) {
    if (!e) {
      return false;
    }
  }

  return true;
}

enum class field_tag {
  pure_field,
  constant,
  checksum,
};

template<typename... Ts>
class description {
  template<typename... Ts_, typename... Us>
  friend constexpr auto operator|(description<Ts_...> lhs, description<Us...> rhs) noexcept;

  template<name, bool Is_Signed, std::size_t Width>
  friend constexpr auto descriptor::field(signedness_t<Is_Signed>, width_t<Width>) noexcept(release);

  template<name, typename T, std::size_t Width>
  friend constexpr auto descriptor::constant(T, width_t<Width>) noexcept(release);

  template<name, typename BinaryOp, std::size_t Width, typename Init>
  friend constexpr auto descriptor::checksum(BinaryOp, width_t<Width>, Init, descriptor::all_fields_t) noexcept(release);

public:
  constexpr static auto identifiers = typelist<Ts...>{}
    .transform_const([](auto field_type) { return field_type->identifier; });

  template<typename Serializer, typename... NamedValues>
  [[nodiscard]] constexpr auto instantiate(Serializer &ser, NamedValues... nvs) const noexcept {
    static_assert((requires { named_value{nvs}; } && ...),
                  "`nvs` must be a pack of `named_value` instances");

    auto named_args = (nvs, ...);
    auto first_pass = [&](const auto &field) {
      if constexpr (field.tag == field_tag::pure_field) {
        constexpr auto width = field.width;
        using representation = std::conditional_t<field.is_signed, xint<width>, xuint<width>>;
        auto value = named_args[kw<field.identifier>];
        return kw<field.identifier> = representation{value};
      } else if constexpr (field.tag == field_tag::constant) {
        return kw<field.identifier> = field.value;
      } else if constexpr (field.tag == field_tag::checksum) {
        constexpr auto width = field.width;
        return kw<field.identifier> = xuint<width>{field.init};
      } else {
        static_assert(UPD_ALWAYS_FALSE, "`field` is not a field object");
      }
    };

    auto make_named_tuple = [](auto &&...nvs) { return name_tuple<nvs.identifier...>(std::tuple{UPD_FWD(nvs).value()...}); };
    auto retval = m_fields.transform(first_pass).apply(make_named_tuple);
    auto second_pass = [&](const auto &field) {
      if constexpr (field.tag == field_tag::checksum) {
        constexpr auto identifier = field.identifier;
        auto &[op, init, get_range] = field;
        auto &acc = retval[kw<identifier>];
        auto folder = invoker_iterator{
          [&, op=op](auto x) { acc = op(acc, x); }
        };

        get_range(identifiers).for_each([&](auto id) {
          return retval[kw<id.value>].serialize(ser, folder);
        });
      }
    };

    m_fields.for_each(second_pass);
    return expected{std::move(retval)};
  }

  template<typename InputIt, typename Serializer>
  [[nodiscard]] constexpr auto decode(InputIt src, Serializer &ser) const noexcept(release) {
    auto err = unexpected{};
    auto accumulators = m_fields
      .filter([](auto field_type) { return field_type->tag == field_tag::checksum; })
      .apply([](auto &&... checksum_fields) {
          return name_tuple<checksum_fields.identifier...>(std::tuple{checksum_fields.init...});
        }
      );

    auto first_pass = [&](const auto &field) {
      if constexpr (field.tag == field_tag::checksum) {
        const auto &[op, init, get_range] = field;
        auto &acc = accumulators[kw<field.identifier>];
        auto range = get_range(identifiers);
        auto accumulate = [&, &op = op](auto identifier, const auto &value) {
          constexpr auto p = [=](auto id_type) { return auto_constant<id_type->value == identifier>{}; };
          if constexpr (range.filter(p).size() > 0) {
            acc = op(acc, value);
          }
        };

        return accumulate;
      }
    };
    
    auto checksum_accs = m_fields.transform(first_pass, filter_void);
    auto second_pass = [&](const auto &field) {
      constexpr auto identifier = field.identifier;
      if constexpr (field.tag == field_tag::pure_field) {
        constexpr auto width = field.width;
        using representation = std::conditional_t<field.is_signed, xint<width>, xuint<width>>;
        auto decorated_src = transformer_iterator{
          detail::iterator_reference{src},
          [&](const auto &value) {
            checksum_accs.for_each([&](auto acc) { acc(auto_constant<identifier>{}, value); });
            return value;
          }
        };
        return kw<identifier> = deserialize_into_xinteger<representation>(decorated_src, ser);
      } else if constexpr (field.tag == field_tag::constant) {
        constexpr auto width = field.width;
        using representation = std::conditional_t<field.is_signed, xint<width>, xuint<width>>;
        auto decorated_src = transformer_iterator{
          detail::iterator_reference{src},
          [&](const auto &value) {
            checksum_accs.for_each([&](auto acc) { acc(auto_constant<identifier>{}, value); });
            return value;
          }
        };

        auto received = deserialize_into_xinteger<representation>(decorated_src, ser);
        if (!err && received != field.value) {
          err = error::constant_mismatch;
        }
      } else if constexpr (field.tag == field_tag::checksum) {
        constexpr auto width = field.width;
        using representation = std::conditional_t<field.is_signed, xint<width>, xuint<width>>;
        auto received = deserialize_into_xinteger<representation>(detail::iterator_reference{src}, ser);
        if (!err && accumulators[kw<identifier>] != received) {
          err = error::checksum_mismatch;
        }
      } else {
        static_assert(UPD_ALWAYS_FALSE, "`field.tag` is not a valid `field_tag` enumerator");
      }
    };

    auto retval = m_fields
      .transform(second_pass, filter_void)
      .apply([](auto &&... nvs) { return name_tuple<nvs.identifier...>(std::tuple{UPD_FWD(nvs).value()...}); });
    return err ? err : expected{std::move(retval)};
  }

private:
  explicit constexpr description(tuple<Ts...> fields) : m_fields{std::move(fields)} {}

  tuple<Ts...> m_fields;
};

template<typename... Ts, typename... Us>
[[nodiscard]] constexpr inline auto operator|(description<Ts...> lhs, description<Us...> rhs) noexcept {
  auto fields = std::move(lhs.m_fields) + std::move(rhs.m_fields);
  auto get_identifier = [](auto field_type) { return field_type->identifier; };

  // For each identifier of the merged description, we count how often it
  // appears. If the total is not equal to the number of fields, we know that
  // there are duplicate identifiers.
  auto self_comparison_count = fields.type_only()
                                   .transform_const(get_identifier)
                                   .square()
                                   .transform(unpack | equal_to)
                                   .fold_const(auto_constant<std::size_t{0}>{}, plus);

  static_assert(self_comparison_count == fields.size(), "Merging these descriptions would result in duplicate IDs");

  return description{std::move(fields)};
}

} // namespace upd

namespace upd::descriptor {

template<name Identifier, bool Signedness, std::size_t Width>
struct field_t {
  constexpr static auto tag = field_tag::pure_field;
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;
};

template<name Identifier, bool Signedness, std::size_t Width>
[[nodiscard]] constexpr auto field(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
  auto retval = field_t<Identifier, Signedness, Width>{};
  return description{tuple{retval}};
}

template<name Identifier, std::size_t Width>
struct constant_t {
  constexpr static auto tag = field_tag::constant;
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = false;
  constexpr static auto width = Width;

  xuint<width> value;
};

template<name Identifier, typename T, std::size_t Width>
[[nodiscard]] constexpr auto constant(T n, width_t<Width>) noexcept(release) {
  auto retval = constant_t<Identifier, Width>{n};
  return description{tuple{retval}};
}

template<name Identifier, typename BinaryOp, std::size_t Width, typename Init, typename GetRange>
struct checksum_t {
  constexpr static auto tag = field_tag::checksum;
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = false;
  constexpr static auto width = Width;

  BinaryOp op;
  Init init;
  GetRange get_range;
};

template<name Identifier, typename BinaryOp, std::size_t Width, typename Init>
[[nodiscard]] constexpr auto checksum(BinaryOp op, width_t<Width>, Init init, all_fields_t) noexcept(release) {
  auto get_range = [](auto identifiers) { return identifiers.filter([](auto id_type) { return id_type->value != Identifier; }); };
  auto retval = checksum_t<Identifier, BinaryOp, Width,
       Init, decltype(get_range)>{std::move(op), init, get_range};
  
  return description{tuple{retval}};
}

} // namespace upd::descriptor

namespace upd::literals {

[[nodiscard]] constexpr inline auto operator""_h(const char *str, std::size_t size) noexcept -> std::size_t {
  constexpr auto numlim = std::numeric_limits<char>{};
  constexpr auto min = std::intmax_t{numlim.min()};
  constexpr auto max = std::intmax_t{numlim.max()};

  auto retval = std::size_t{0};

  // `std::hash` cannot be invoked in constant expression, so here is the poor man's hashing function for the moment
  for (std::size_t i = 0; i < size; ++i) {
    retval += str[i] - min;
    retval *= max - min;
  }

  return retval;
}

} // namespace upd::literals
