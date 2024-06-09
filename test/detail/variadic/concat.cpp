#include <mutex>
#include <tuple>
#include <type_traits>

#include <upd/detail/variadic/concat.hpp>
#include <upd/detail/variadic/leaf.hpp>

using namespace upd::detail::variadic;

static_assert(std::is_same_v<concat_t<std::tuple<int, bool, std::mutex>, std::tuple<char, long>>,
                             std::tuple<int, bool, std::mutex, char, long>>);
