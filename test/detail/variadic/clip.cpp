#include <mutex>
#include <tuple>
#include <type_traits>

#include <upd/detail/variadic/clip.hpp>
#include <upd/detail/variadic/leaf.hpp>

using namespace upd::detail::variadic;

static_assert(std::is_same_v<clip_t<std::tuple<int, bool, std::mutex>, 0, 2>, std::tuple<int, bool>>);

static_assert(std::is_same_v<clip_t<std::tuple<int, bool, std::mutex>, 1, 3>, std::tuple<bool, std::mutex>>);

static_assert(std::is_same_v<clip_t<std::tuple<int, bool, std::mutex>, 1, 1>, std::tuple<>>);
