#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <type_traits>
#include <utility>
#include <variant>

#include "template_traits.hpp"
#include "tuple/has_type.hpp"
#include "tuple/typelist.hpp"
#include "upd.hpp"

namespace upd {

struct no_error {};

struct not_matching_deduction {
  const char *identifier;
  std::intmax_t actual;
  std::intmax_t deduced;
};

struct invalid_code_in_one_of {
  const char *identifier;
  std::intmax_t code;
};

struct negative_repetition_count {
  const char *identifier;
  std::intmax_t count;
};

struct repeated_beyond_max {
  const char *identifier;
  std::uintmax_t count;
  std::size_t max;
};

using error_data_types = typelist2_t<no_error,
                                     not_matching_deduction,
                                     invalid_code_in_one_of,
                                     negative_repetition_count,
                                     repeated_beyond_max>;

template<typename T>
concept error_data = has_type<T>(error_data_types{});

class error {
  using data_type = instantiate_variadic<std::variant, error_data_types>;

public:
  constexpr error() noexcept(release) = default;
  constexpr error(const error &) noexcept(release) = default;
  constexpr error(error &&) noexcept(release) = default;

  template<error_data ErrorData>
  constexpr error(ErrorData &&err_data) noexcept(release) : m_data{UPD_FWD(err_data)} {}

  constexpr auto operator=(const error &) noexcept(release) -> error & = default;
  constexpr auto operator=(error &&) noexcept(release) -> error & = default;

  template<error_data ErrorData>
  constexpr auto operator=(ErrorData &&err_data) noexcept(release) -> error & {
    m_data = UPD_FWD(err_data);
    return *this;
  }

  [[nodiscard]] constexpr operator bool() const noexcept(release) { return !std::holds_alternative<no_error>(m_data); }

  template<typename F>
  constexpr auto visit(F &&f) const -> decltype(auto) {
    return std::visit(UPD_FWD(f), m_data);
  }

private:
  data_type m_data;
};

template<typename T>
using result = std::expected<T, error>;

template<typename T>
[[nodiscard]] constexpr auto result_if_no_error(T &&x, const error &err) -> result<std::remove_cvref_t<T>> {
  if (err) {
    return std::unexpected{err};
  } else {
    return UPD_FWD(x);
  }
}

template<typename T>
[[nodiscard]] constexpr auto result_if_no_error(T &&x, error &&err) -> result<std::remove_cvref_t<T>> {
  if (err) {
    return std::unexpected{std::move(err)};
  } else {
    return UPD_FWD(x);
  }
}

} // namespace upd

template<>
struct std::formatter<upd::no_error> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(upd::no_error, std::format_context &ctx) { return std::format_to(ctx.out(), "No error"); }
};

template<>
struct std::formatter<upd::not_matching_deduction> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const upd::not_matching_deduction &err, std::format_context &ctx) {
    return std::format_to(ctx.out(),
                          "'{}' field actual and deduced value do not match ('{} (0x{:x})' vs '{} (0x{:x})')",
                          err.identifier,
                          err.actual,
                          err.actual,
                          err.deduced,
                          err.deduced);
  }
};

template<>
struct std::formatter<upd::invalid_code_in_one_of> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const upd::invalid_code_in_one_of &err, std::format_context &ctx) {
    return std::format_to(
        ctx.out(), "{} is not a code of any alternative in one-of field '{}'", err.code, err.identifier);
  }
};

template<>
struct std::formatter<upd::negative_repetition_count> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const upd::negative_repetition_count &err, std::format_context &ctx) {
    return std::format_to(
        ctx.out(), "Negative repetition count {} found for repetition field '{}'", err.count, err.identifier);
  }
};

template<>
struct std::formatter<upd::repeated_beyond_max> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const upd::repeated_beyond_max &err, std::format_context &ctx) {
    return std::format_to(
        ctx.out(), "Field repeated {} time in '{}', beyond the maximum limit ({})", err.count, err.identifier, err.max);
  }
};

template<>
struct std::formatter<upd::error> {
  constexpr static auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const upd::error &err, std::format_context &ctx) {
    auto format_error_data = [&](const auto &err_data) { return std::format_to(ctx.out(), "{}", err_data); };

    return err.visit(format_error_data);
  }
};
