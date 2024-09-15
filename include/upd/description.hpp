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
#include "detail/variadic/diff.hpp"
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

template<auto, bool, std::size_t>
struct field_t;

template<auto, std::size_t>
struct constant_t;

} // namespace upd::descriptor

namespace upd::detail {

template<typename Iter>
class iterator_reference {
public:
  using iterator_type = Iter;
  using iterator_category = std::input_iterator_tag;
  using value_type = typename std::iterator_traits<Iter>::value_type;
  using difference_type = typename std::iterator_traits<Iter>::difference_type;
  using pointer = typename std::iterator_traits<Iter>::pointer;
  using reference = typename std::iterator_traits<Iter>::reference;

  constexpr explicit iterator_reference(iterator_type &iter) : m_cache{*iter}, m_iter{iter} { ++iter; }

  [[nodiscard]] constexpr auto operator*() const -> value_type { return m_cache; }

  [[nodiscard]] constexpr auto operator++() -> iterator_reference & {
    m_cache = *m_iter;
    ++m_iter.get();
    return *this;
  }

  [[nodiscard]] constexpr auto operator++() const -> const iterator_reference & {
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

enum class error {
  none,
};

template<typename E>
class unexpected {
public:
  constexpr explicit unexpected(E e) noexcept : m_error{std::move(e)} {}

  [[nodiscard]] constexpr auto error() noexcept -> E { return m_error; }

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

template<auto, bool Is_Signed, std::size_t Width>
constexpr auto field(signedness_t<Is_Signed>, width_t<Width>) noexcept(release);

template<auto, typename T, std::size_t Width>
constexpr auto constant(T, width_t<Width>) noexcept(release);

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
};

template<typename... Ts>
class description {
  template<typename... Ts_, typename... Us>
  friend constexpr auto operator|(description<Ts_...> lhs, description<Us...> rhs) noexcept;

  template<auto, bool Is_Signed, std::size_t Width>
  friend constexpr auto descriptor::field(signedness_t<Is_Signed>, width_t<Width>) noexcept(release);

  template<auto, typename T, std::size_t Width>
  friend constexpr auto descriptor::constant(T, width_t<Width>) noexcept(release);

public:
  template<typename... NamedValues>
  [[nodiscard]] constexpr auto instantiate(NamedValues... nvs) const noexcept {
    static_assert((detail::is_instance_of_v<NamedValues, named_value> && ...),
                  "`nvs` must be a pack of `named_value` instances");

    auto named_args = (nvs, ...);
    auto impl = [&](const auto &field) {
      if constexpr (field.tag == field_tag::pure_field) {
        constexpr auto width = field.width;
        using representation = std::conditional_t<field.is_signed, xint<width>, xuint<width>>;
        auto value = named_args[kw<field.identifier>];
        return kw<field.identifier> = representation{value};
      } else if constexpr (field.tag == field_tag::constant) {
        return kw<field.identifier> = field.value;
      } else {
        static_assert(UPD_ALWAYS_FALSE, "`field` is not a field object");
      }
    };

    auto make_named_tuple = [](auto &&...nvs) { return (nvs, ...); };

    return expected{m_fields.transform(impl).apply(make_named_tuple)};
  }

  template<typename InputIt, typename Serializer>
  [[nodiscard]] constexpr auto decode(InputIt src, Serializer &ser) const noexcept(release) {
    auto impl = [&](const auto &field) {
      if constexpr (field.tag == field_tag::pure_field) {
        constexpr auto width = field.width;
        using representation = std::conditional_t<field.is_signed, xint<width>, xuint<width>>;
        return kw<field.identifier> = deserialize_into_xinteger<representation>(detail::iterator_reference{src}, ser);
      } else if constexpr (field.tag == field_tag::constant) {
        return kw<field.identifier> = field.value;
      } else {
        static_assert(UPD_ALWAYS_FALSE, "`field` is not a field object");
      }
    };

    auto make_named_tuple = [](auto &&...nvs) { return (nvs, ...); };

    auto retval = m_fields.transform(impl).apply(make_named_tuple);
    return expected{std::move(retval)};
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
                                   .transform(equal_to)
                                   .fold_const(auto_constant<std::size_t{0}>{}, plus);

  static_assert(self_comparison_count == fields.size(), "Merging these descriptions would result in duplicate IDs");

  return description{std::move(fields)};
}

} // namespace upd

namespace upd::descriptor {

template<auto Identifier, bool Signedness, std::size_t Width>
struct field_t {
  constexpr static auto tag = field_tag::pure_field;
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = Signedness;
  constexpr static auto width = Width;
};

template<auto Identifier, bool Signedness, std::size_t Width>
[[nodiscard]] constexpr auto field(signedness_t<Signedness>, width_t<Width>) noexcept(release) {
  auto retval = field_t<Identifier, Signedness, Width>{};
  return description{tuple{retval}};
}

template<auto Identifier, std::size_t Width>
struct constant_t {
  constexpr static auto tag = field_tag::constant;
  constexpr static auto identifier = Identifier;
  constexpr static auto is_signed = false;
  constexpr static auto width = Width;

  xuint<width> value;
};

template<auto Identifier, typename T, std::size_t Width>
[[nodiscard]] constexpr auto constant(T n, width_t<Width>) noexcept(release) {
  auto retval = constant_t<Identifier, Width>{};
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
