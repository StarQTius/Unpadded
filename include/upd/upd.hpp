#pragma once

#define UPD_FWD(x) static_cast<decltype(x) &&>(x)
#define UPD_PACK(...) __VA_ARGS__

namespace upd {}

template<typename>
struct upd_extension; // IWYU pragma: keep
