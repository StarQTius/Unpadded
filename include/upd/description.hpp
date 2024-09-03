#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <tuple>
#include <variant>
#include <type_traits>

#include "detail/has_value_member.hpp"
#include "detail/always_false.hpp"
#include "detail/integral_constant.hpp"
#include "detail/variadic/map.hpp"
#include "detail/variadic/product.hpp"
#include "detail/variadic/diff.hpp"
#include "detail/variadic/to_array.hpp"
#include "detail/variadic/equals.hpp"
#include "upd.hpp"
#include "token.hpp"
#include "named_value.hpp"
#include "integer.hpp"

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

  constexpr explicit iterator_reference(iterator_type &iter):
    m_cache{*iter},
    m_iter{iter}
  {
    ++iter;
  }

  [[nodiscard]] constexpr auto operator*() const -> value_type {
    return m_cache;
  }

  [[nodiscard]] constexpr auto operator++() -> iterator_reference& {
    m_cache = *m_iter;
    ++m_iter.get();
    return *this;
  }

  [[nodiscard]] constexpr auto operator++() const -> const iterator_reference& {
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
  constexpr explicit unexpected(E e) noexcept:
    m_error{std::move(e)}
  {}
  
  [[nodiscard]] constexpr auto error() noexcept -> E {
    return m_error;
  }

private:
  E m_error;
};

template<typename T, typename E = error>
class expected {
public:
  constexpr explicit expected(T x) noexcept:
    m_value_or_error{std::in_place_index<0>, std::move(x)}
  {}
  
  constexpr expected(unexpected<E> e) noexcept:
    m_value_or_error{std::in_place_index<1>, e.error()}
  {}

  [[nodiscard]] constexpr operator bool() const noexcept {
    return value_if() != nullptr;
  }

  [[nodiscard]] constexpr auto operator*() noexcept -> T& {
    return value();
  }

  [[nodiscard]] constexpr auto operator*() const noexcept -> const T& {
    return value();
  }

  [[nodiscard]] constexpr auto operator->() noexcept -> T* {
    return &value();
  }

  [[nodiscard]] constexpr auto operator->() const noexcept -> const T* {
    return &value();
  }

  [[nodiscard]] constexpr auto value() & noexcept -> T& {
    auto *v = value_if();
    UPD_ASSERT(v != nullptr);
    
    return *v;
  }

  [[nodiscard]] constexpr auto value() const & noexcept -> const T& {
    const auto *v = value_if();
    UPD_ASSERT(v != nullptr);
    
    return *v;
  }

  [[nodiscard]] constexpr auto value() && noexcept -> T&& {
    auto *v = value_if();
    UPD_ASSERT(v != nullptr);
    
    return std::move(*v);
  }

  [[nodiscard]] constexpr auto error() & noexcept -> E& {
    auto *e = error_if();
    UPD_ASSERT(e != nullptr);
    
    return *e;
  }

  [[nodiscard]] constexpr auto error() const & noexcept -> const E& {
    const auto *e = error_if();
    UPD_ASSERT(e != nullptr);
    
    return *e;
  }

  [[nodiscard]] constexpr auto error() && noexcept -> E&& {
    auto *e = error_if();
    UPD_ASSERT(e != nullptr);
    
    return std::move(*e);
  }

  [[nodiscard]] constexpr auto value_if() noexcept -> T* {
    return std::get_if<0>(&m_value_or_error);
  }

  [[nodiscard]] constexpr auto value_if() const noexcept -> const T* {
    return std::get_if<0>(&m_value_or_error);
  }

  [[nodiscard]] constexpr auto error_if() noexcept -> E* {
    return std::get_if<1>(&m_value_or_error);
  }

  [[nodiscard]] constexpr auto error_if() const noexcept -> const E* {
    return std::get_if<1>(&m_value_or_error);
  }

private:
  std::variant<T, E> m_value_or_error;

};

} // namespace upd

namespace upd::descriptor {

template<auto, bool Is_Signed, std::size_t Width>
constexpr auto field(signedness_t<Is_Signed>, width_t<Width>) noexcept;

} // namespace upd::descriptor

namespace upd {

template<typename Range>
[[nodiscard]] constexpr auto all_of(const Range &range) noexcept -> bool {
  for (const auto &e: range) {
    if (!e) {
      return false;
    }
  }

  return true;
}

template<typename... Ts>
class description {
  template<typename... Ts_, typename... Us>
  friend constexpr auto operator|(description<Ts_...> lhs, description<Us...> rhs) noexcept;

  template<auto, bool Is_Signed, std::size_t Width>
  friend constexpr auto descriptor::field(signedness_t<Is_Signed>, width_t<Width>) noexcept;

public:
  constexpr static auto ids = [] { return std::tuple_cat(Ts::ids...); }();

  [[nodiscard]] constexpr auto fields() const & -> std::tuple<Ts...> { return m_fields; }

  [[nodiscard]] constexpr auto fields() && -> std::tuple<Ts...> { return std::move(m_fields); }

  template<typename... NamedValues>
  [[nodiscard]] constexpr auto instantiate(NamedValues... nvs) const noexcept {
    using identifier_types = std::decay_t<decltype(ids)>;
    using value_name_types = std::tuple<detail::integral_constant_t<nvs.identifier>...>;

    static_assert((detail::is_instance_of_v<NamedValues, named_value> && ...), "`nvs` must be a pack of `named_value` instances");
    static_assert(detail::variadic::equals_v<identifier_types, value_name_types>, "`nvs.identifier...` must match the content of `ids` in order");

    return expected{(std::move(nvs).map([](auto n){ return typename Ts::underlying{n}; }), ...)};
  }

  template<typename InputIt, typename Serializer>
  [[nodiscard]] constexpr auto decode(InputIt src, Serializer &ser) const {
    auto impl = [&](auto... identifiers) {
      return ((kw<identifiers.value> = deserialize_into_xinteger<typename Ts::underlying>(detail::iterator_reference{src}, ser)), ...);
    };

    auto retval = std::apply(impl, ids);
    return expected{std::move(retval)};
  }

private:
  explicit constexpr description(Ts... xs) : m_fields{UPD_FWD(xs)...} {}

  std::tuple<Ts...> m_fields;
};

template<typename... Ts, typename... Us>
[[nodiscard]] constexpr inline auto operator|(description<Ts...> lhs, description<Us...> rhs) noexcept {
  auto fields = std::tuple_cat(std::move(lhs.m_fields), std::move(rhs.m_fields));
  auto get_ids = [](auto x) { return x.ids; };

  using id_ts = detail::variadic::flatmapf_t<decltype(fields), decltype(get_ids)>;
  using id_squared_ts = detail::variadic::product_t<id_ts, id_ts>;
  using is_same_ts = detail::variadic::binmap_t<id_squared_ts, std::is_same>;
  constexpr auto are_unique = detail::variadic::sum_v<is_same_ts> == std::tuple_size_v<id_ts>;

  if constexpr (are_unique) {
    auto make_description = [](auto... fields) { return description{std::move(fields)...}; };
    return std::apply(make_description, std::move(fields));
  } else {
    static_assert(UPD_ALWAYS_FALSE, "Merging these descriptions would result in duplicate IDs");
  }
}

} // namespace upd

namespace upd::descriptor {

template<typename Id, typename Is_Signed, typename Width>
struct field_t {
  constexpr static auto id = Id::value;
  constexpr static auto is_signed = Is_Signed::value;
  constexpr static auto width = Width::value;
  constexpr static auto ids = std::tuple<Id>{};

  using underlying = xinteger<width, std::uintmax_t>;
};

template<auto Id, bool Is_Signed, std::size_t Width>
[[nodiscard]] constexpr auto field(signedness_t<Is_Signed>, width_t<Width>) noexcept {
  using id = detail::integral_constant_t<Id>;
  using is_signed = detail::integral_constant_t<Is_Signed>;
  using width = detail::integral_constant_t<Width>;

  auto retval = field_t<id, is_signed, width>{};
  return description{retval};
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
