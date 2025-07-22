#pragma once

namespace upd {

#if defined(UPD_DEBUG)

constexpr auto release = false;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)                                                                                                \
  if (!(__VA_ARGS__)) {                                                                                                \
    throw std::exception{};                                                                                            \
  }

#else // defined(UPD_DEBUG)

constexpr auto release = true;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...)

#endif // defined(UPD_DEBUG)

} // namespace upd

// NOLINTBEGIN

#define UPD_FWD(x) static_cast<decltype(x) &&>(x)
#define UPD_PACK(...) __VA_ARGS__
#define UPD_SCOPE_OPERATOR(LHS, RHS) LHS::RHS

namespace upd::detail {};

// NOLINTEND
