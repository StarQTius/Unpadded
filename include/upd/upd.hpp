#pragma once

#include <expected>
#include <format>
#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace upd {

#if defined(UPD_DEBUG)

constexpr auto release = false;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)                                                        \
  if (!(__VA_ARGS__)) {                                                        \
    throw std::exception{};                                                    \
  }

#else // defined(UPD_DEBUG)

constexpr auto release = true;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)

#endif // defined(UPD_DEBUG)

} // namespace upd

// NOLINTBEGIN

#define UPD_ESCAPE(...) __VA_ARGS__
#define UPD_FWD(x) static_cast<decltype(x) &&>(x)
#define UPD_INVOKE(INVOCABLE, ...) ((INVOCABLE)(__VA_ARGS__))
#define UPD_INVOKE_TEMPLATE(INVOCABLE, TARG_LIST, ...)                         \
  ((INVOCABLE.template operator()<UPD_ESCAPE TARG_LIST>)(__VA_ARGS__))

// NOLINTEND

template<typename T>
struct std::formatter<std::reference_wrapper<T>> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(std::reference_wrapper<T> ref, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "[std::reference_wrapper: {}]",
                          reinterpret_cast<const void *>(&ref.get()));
  }
};

template<typename Enum>
  requires std::is_enum_v<Enum>
struct std::formatter<Enum> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(Enum e, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{}", std::to_underlying(e));
  }
};

template<typename... Ts>
struct std::formatter<std::variant<Ts...>> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto
  format(const std::variant<Ts...> &one_of_values, std::format_context &ctx) {
    auto format_alt = [&](const auto &alt) {
      return std::format_to(ctx.out(), "{}", alt);
    };
    return std::visit(format_alt, one_of_values);
  }
};

template<>
struct std::formatter<std::monostate> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(std::monostate, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "<monostate>");
  }
};

template<typename T, typename E>
struct std::formatter<std::expected<T, E>> {
  constexpr static auto parse(std::format_parse_context &ctx) {
    return ctx.begin();
  }

  static auto format(const std::expected<T, E> &res, std::format_context &ctx) {
    if (!res) {
      return std::format_to(ctx.out(), "{}", res.error());
    }

    if constexpr (std::is_void_v<T>) {
      return std::format_to(ctx.out(), "<void>");
    } else {
      return std::format_to(ctx.out(), "{}", *res);
    }
  }
};
