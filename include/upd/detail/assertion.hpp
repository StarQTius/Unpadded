#pragma once

#if !defined(UPD_ASSERT)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UPD_ASSERT(...) REQUIRE(__VA_ARGS__)

#endif // !defined(UPD_ASSERT)
